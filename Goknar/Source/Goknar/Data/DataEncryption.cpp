#include "pch.h"

#include "DataEncryption.h"

#include "Goknar/Log.h"

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "zlib.h"

namespace
{
	constexpr uint32_t kPakMagic = 0x4B504B47u; // "GKPK"
	constexpr uint32_t kPakVersion = 1u;
	constexpr uint32_t kCompressionNone = 0u;
	constexpr uint32_t kCompressionZlib = 1u;

	constexpr uint64_t kHeaderMountPointKey = 0x1A0B9D27F31C4E55ull;
	constexpr uint64_t kHeaderEntryCountKey = 0x4BD3F9A1082E6C77ull;
	constexpr uint64_t kHeaderIndexOffsetKey = 0x90F61E3C57A4B2D1ull;
	constexpr uint64_t kHeaderIndexSizeKey = 0x7C24AE198EF05233ull;

	constexpr uint64_t kPrimaryPathSeed = 14695981039346656037ull;
	constexpr uint64_t kSecondaryPathSeed = 1099511628211ull ^ 0xC6A4A7935BD1E995ull;
	constexpr uint64_t kEncryptionSeedA = 0x9E3779B97F4A7C15ull;
	constexpr uint64_t kEncryptionSeedB = 0xD1B54A32D192ED03ull;

#pragma pack(push, 1)
	struct PakHeader
	{
		uint32_t magic{ 0u };
		uint32_t version{ 0u };
		uint64_t headerNonce{ 0u };
		uint64_t mountPointLength{ 0u };
		uint64_t entryCount{ 0u };
		uint64_t indexOffset{ 0u };
		uint64_t indexSize{ 0u };
	};

	struct PakEntryRecord
	{
		uint64_t primaryPathHash{ 0u };
		uint64_t secondaryPathHash{ 0u };
		uint64_t dataOffset{ 0u };
		uint64_t storedSize{ 0u };
		uint64_t originalSize{ 0u };
		uint64_t dataCrc32{ 0u };
		uint64_t fileNonce{ 0u };
		uint64_t compressionMethod{ 0u };
	};
#pragma pack(pop)

	struct PathKey
	{
		uint64_t primary{ 0u };
		uint64_t secondary{ 0u };

		bool operator==(const PathKey& other) const
		{
			return primary == other.primary && secondary == other.secondary;
		}
	};

	struct PathKeyHasher
	{
		size_t operator()(const PathKey& key) const
		{
			return static_cast<size_t>(key.primary ^ (key.secondary + 0x9E3779B97F4A7C15ull + (key.primary << 6) + (key.primary >> 2)));
		}
	};

	struct MountedPakEntry
	{
		PathKey key{};
		uint64_t dataOffset{ 0u };
		uint64_t storedSize{ 0u };
		uint64_t originalSize{ 0u };
		uint32_t dataCrc32{ 0u };
		uint64_t fileNonce{ 0u };
		uint32_t compressionMethod{ kCompressionNone };
	};

	struct MountedPak
	{
		std::string pakPath{};
		std::string mountPoint{};
		std::vector<MountedPakEntry> entries{};
		std::unordered_map<PathKey, size_t, PathKeyHasher> entryLookup{};
	};

	struct PakBuildEntry
	{
		std::string relativePath{};
		PathKey key{};
		uint64_t dataOffset{ 0u };
		uint64_t storedSize{ 0u };
		uint64_t originalSize{ 0u };
		uint32_t dataCrc32{ 0u };
		uint64_t fileNonce{ 0u };
		uint32_t compressionMethod{ kCompressionNone };
		std::vector<uint8_t> storedData{};
	};

	std::mutex mountedPakMutex;
	std::vector<MountedPak> mountedPaks;
	bool publishedMountScanAttempted = false;

	std::string NormalizeMountPoint(const std::string& mountPoint)
	{
		std::string normalizedMountPoint = DataEncryption::NormalizePath(mountPoint);
		if (!normalizedMountPoint.empty() && normalizedMountPoint.back() != '/')
		{
			normalizedMountPoint += '/';
		}

		return normalizedMountPoint;
	}

	uint64_t RotateLeft(const uint64_t value, const uint32_t shift)
	{
		return (value << shift) | (value >> (64u - shift));
	}

	uint64_t Mix64(uint64_t value)
	{
		value += 0x9E3779B97F4A7C15ull;
		value = (value ^ (value >> 30u)) * 0xBF58476D1CE4E5B9ull;
		value = (value ^ (value >> 27u)) * 0x94D049BB133111EBull;
		return value ^ (value >> 31u);
	}

	uint64_t Obfuscate64(const uint64_t value, const uint64_t key)
	{
		return value ^ Mix64(key);
	}

	uint64_t Deobfuscate64(const uint64_t value, const uint64_t key)
	{
		return Obfuscate64(value, key);
	}

	std::string NormalizeForHashing(const std::string& path)
	{
		std::string normalizedPath = DataEncryption::NormalizePath(path);
		if (!normalizedPath.empty() && normalizedPath.front() == '/')
		{
			normalizedPath.erase(normalizedPath.begin());
		}

		return normalizedPath;
	}

	uint64_t HashString(const std::string& value, uint64_t seed)
	{
		uint64_t hash = seed;
		for (const unsigned char character : value)
		{
			hash ^= static_cast<uint64_t>(character);
			hash *= 1099511628211ull;
		}

		return hash;
	}

	PathKey ComputePathKey(const std::string& relativePath)
	{
		const std::string normalizedPath = NormalizeForHashing(relativePath);
		std::string reversedPath = normalizedPath;
		std::reverse(reversedPath.begin(), reversedPath.end());

		PathKey key;
		key.primary = HashString(normalizedPath, kPrimaryPathSeed);
		key.secondary = HashString(reversedPath, kSecondaryPathSeed);
		return key;
	}

	uint64_t ComputePakNonce(const std::string& mountPoint, const uint64_t entryCount)
	{
		return Mix64(HashString(mountPoint, kPrimaryPathSeed) ^ RotateLeft(entryCount + 0xA24BAED4963EE407ull, 17u));
	}

	uint64_t ComputeFileNonce(const PathKey& key, const uint64_t originalSize)
	{
		return Mix64(key.primary ^ RotateLeft(key.secondary, 13u) ^ RotateLeft(originalSize + 0x51EDCEB95A1C7B49ull, 29u));
	}

	void EncryptBuffer(uint8_t* data, const size_t size, const PathKey& key, const uint64_t fileNonce)
	{
		uint64_t state = Mix64(kEncryptionSeedA ^ key.primary ^ RotateLeft(key.secondary, 7u) ^ fileNonce);
		for (size_t byteIndex = 0; byteIndex < size; ++byteIndex)
		{
			if ((byteIndex & 7u) == 0u)
			{
				state = Mix64(state + kEncryptionSeedB + static_cast<uint64_t>(byteIndex));
			}

			const uint8_t keyByte = static_cast<uint8_t>((state >> ((byteIndex & 7u) * 8u)) & 0xFFu);
			data[byteIndex] ^= keyByte;
		}
	}

	bool ReadFileBytesFromDisk(const std::string& filePath, std::vector<uint8_t>& outData)
	{
		outData.clear();

		std::ifstream file(filePath, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			return false;
		}

		const std::streamsize fileSize = file.tellg();
		if (fileSize < 0)
		{
			return false;
		}

		file.seekg(0, std::ios::beg);
		outData.resize(static_cast<size_t>(fileSize));
		if (fileSize == 0)
		{
			return true;
		}

		return file.read(reinterpret_cast<char*>(outData.data()), fileSize).good();
	}

	bool TryReadFromMountedPaks(const std::string& filePath, std::vector<uint8_t>& outData, bool& pathMatchesMountedMountPoint)
	{
		pathMatchesMountedMountPoint = false;
		const std::string normalizedPath = DataEncryption::NormalizePath(filePath);

		std::lock_guard<std::mutex> lock(mountedPakMutex);
		for (const MountedPak& mountedPak : mountedPaks)
		{
			std::string relativePath;
			const std::string prefixedMountPoint = "/" + mountedPak.mountPoint;
			if (normalizedPath.rfind(mountedPak.mountPoint, 0) == 0)
			{
				relativePath = normalizedPath.substr(mountedPak.mountPoint.size());
			}
			else
			{
				const size_t mountPointPosition = normalizedPath.find(prefixedMountPoint);
				if (mountPointPosition == std::string::npos)
				{
					continue;
				}

				relativePath = normalizedPath.substr(mountPointPosition + prefixedMountPoint.size());
			}

			pathMatchesMountedMountPoint = true;
			const PathKey key = ComputePathKey(relativePath);
			const auto entryIterator = mountedPak.entryLookup.find(key);
			if (entryIterator == mountedPak.entryLookup.end())
			{
				continue;
			}

			const MountedPakEntry& entry = mountedPak.entries[entryIterator->second];
			std::ifstream pakFile(mountedPak.pakPath, std::ios::binary);
			if (!pakFile.is_open())
			{
				return false;
			}

			pakFile.seekg(static_cast<std::streamoff>(entry.dataOffset), std::ios::beg);
			if (!pakFile.good())
			{
				return false;
			}

			std::vector<uint8_t> storedData(static_cast<size_t>(entry.storedSize));
			if (entry.storedSize > 0u)
			{
				pakFile.read(reinterpret_cast<char*>(storedData.data()), static_cast<std::streamsize>(entry.storedSize));
				if (!pakFile.good())
				{
					return false;
				}
			}

			EncryptBuffer(storedData.data(), storedData.size(), key, entry.fileNonce);

			if (entry.compressionMethod == kCompressionZlib)
			{
				outData.resize(static_cast<size_t>(entry.originalSize));
				uLongf destinationSize = static_cast<uLongf>(outData.size());
				const int uncompressResult = uncompress(
					outData.data(),
					&destinationSize,
					storedData.data(),
					static_cast<uLong>(storedData.size()));
				if (uncompressResult != Z_OK)
				{
					outData.clear();
					return false;
				}

				outData.resize(static_cast<size_t>(destinationSize));
			}
			else
			{
				outData = std::move(storedData);
			}

			const uint32_t crc = crc32(0L, Z_NULL, 0);
			const uint32_t calculatedCrc = crc32(crc, outData.data(), static_cast<uInt>(outData.size()));
			if (calculatedCrc != entry.dataCrc32)
			{
				outData.clear();
				return false;
			}

			return true;
		}

		return false;
	}

	std::string GetDefaultMountPointForDirectory(const std::string& sourceDirectory)
	{
		const std::filesystem::path directoryPath(sourceDirectory);
		return NormalizeMountPoint(directoryPath.filename().generic_string());
	}
}

bool DataEncryption::CreatePakFromDirectory(const std::string& sourceDirectory, const std::string& pakPath, const std::string& mountPoint)
{
	std::error_code errorCode;
	const std::filesystem::path sourcePath = std::filesystem::weakly_canonical(std::filesystem::path(sourceDirectory), errorCode);
	if (errorCode || !std::filesystem::exists(sourcePath) || !std::filesystem::is_directory(sourcePath))
	{
		GOKNAR_CORE_ERROR("Failed to package directory %s. The source directory could not be resolved.", sourceDirectory.c_str());
		return false;
	}

	const std::string normalizedMountPoint = NormalizeMountPoint(mountPoint.empty() ? GetDefaultMountPointForDirectory(sourceDirectory) : mountPoint);
	if (normalizedMountPoint.empty())
	{
		GOKNAR_CORE_ERROR("Failed to package directory %s. The mount point is empty.", sourceDirectory.c_str());
		return false;
	}

	std::vector<PakBuildEntry> buildEntries;
	std::unordered_map<PathKey, std::string, PathKeyHasher> pathCollisionMap;

	for (const auto& directoryEntry : std::filesystem::recursive_directory_iterator(sourcePath, std::filesystem::directory_options::skip_permission_denied))
	{
		if (!directoryEntry.is_regular_file())
		{
			continue;
		}

		std::vector<uint8_t> fileBytes;
		if (!ReadFileBytesFromDisk(directoryEntry.path().generic_string(), fileBytes))
		{
			GOKNAR_CORE_ERROR("Failed to read %s while creating %s.", directoryEntry.path().generic_string().c_str(), pakPath.c_str());
			return false;
		}

		const std::string relativePath = NormalizeForHashing(std::filesystem::relative(directoryEntry.path(), sourcePath, errorCode).generic_string());
		if (errorCode || relativePath.empty())
		{
			GOKNAR_CORE_ERROR("Failed to resolve the relative path for %s while creating %s.", directoryEntry.path().generic_string().c_str(), pakPath.c_str());
			return false;
		}

		PakBuildEntry buildEntry;
		buildEntry.relativePath = relativePath;
		buildEntry.key = ComputePathKey(relativePath);
		buildEntry.originalSize = static_cast<uint64_t>(fileBytes.size());
		buildEntry.fileNonce = ComputeFileNonce(buildEntry.key, buildEntry.originalSize);
		buildEntry.dataCrc32 = crc32(crc32(0L, Z_NULL, 0), fileBytes.data(), static_cast<uInt>(fileBytes.size()));

		const auto collisionIterator = pathCollisionMap.find(buildEntry.key);
		if (collisionIterator != pathCollisionMap.end() && collisionIterator->second != relativePath)
		{
			GOKNAR_CORE_ERROR("A pak hash collision was detected between %s and %s.", collisionIterator->second.c_str(), relativePath.c_str());
			return false;
		}

		pathCollisionMap[buildEntry.key] = relativePath;

		if (!fileBytes.empty())
		{
			std::vector<uint8_t> compressedData(compressBound(static_cast<uLong>(fileBytes.size())));
			uLongf compressedSize = static_cast<uLongf>(compressedData.size());
			const int compressionResult = compress2(
				compressedData.data(),
				&compressedSize,
				fileBytes.data(),
				static_cast<uLong>(fileBytes.size()),
				Z_BEST_SPEED);

			const bool shouldStoreCompressed = compressionResult == Z_OK && compressedSize > 0u && compressedSize < fileBytes.size();
			if (shouldStoreCompressed)
			{
				compressedData.resize(static_cast<size_t>(compressedSize));
				buildEntry.compressionMethod = kCompressionZlib;
				buildEntry.storedData = std::move(compressedData);
			}
			else
			{
				buildEntry.compressionMethod = kCompressionNone;
				buildEntry.storedData = std::move(fileBytes);
			}

			EncryptBuffer(buildEntry.storedData.data(), buildEntry.storedData.size(), buildEntry.key, buildEntry.fileNonce);
		}

		buildEntry.storedSize = static_cast<uint64_t>(buildEntry.storedData.size());
		buildEntries.emplace_back(std::move(buildEntry));
	}

	std::sort(buildEntries.begin(), buildEntries.end(), [](const PakBuildEntry& left, const PakBuildEntry& right)
		{
			return Mix64(left.key.primary ^ RotateLeft(left.key.secondary, 9u)) <
				Mix64(right.key.primary ^ RotateLeft(right.key.secondary, 9u));
		});

	std::filesystem::path outputPath = std::filesystem::path(pakPath).lexically_normal();
	if (!outputPath.is_absolute())
	{
		outputPath = std::filesystem::absolute(outputPath, errorCode);
	}

	const std::filesystem::path outputParentPath = outputPath.parent_path();
	if (!outputParentPath.empty())
	{
		std::filesystem::create_directories(outputParentPath, errorCode);
	}

	std::ofstream pakFile;
#ifdef GOKNAR_PLATFORM_WINDOWS
	pakFile.open(outputPath.wstring(), std::ios::binary | std::ios::trunc);
#else
	pakFile.open(outputPath, std::ios::binary | std::ios::trunc);
#endif
	if (!pakFile.is_open())
	{
		GOKNAR_CORE_ERROR("Failed to create %s.", outputPath.generic_string().c_str());
		return false;
	}

	const uint64_t headerNonce = ComputePakNonce(normalizedMountPoint, static_cast<uint64_t>(buildEntries.size()));
	PakHeader header{};
	header.magic = kPakMagic;
	header.version = kPakVersion;
	header.headerNonce = headerNonce;
	pakFile.write(reinterpret_cast<const char*>(&header), sizeof(PakHeader));
	pakFile.write(normalizedMountPoint.data(), static_cast<std::streamsize>(normalizedMountPoint.size()));

	for (PakBuildEntry& buildEntry : buildEntries)
	{
		buildEntry.dataOffset = static_cast<uint64_t>(pakFile.tellp());
		if (!buildEntry.storedData.empty())
		{
			pakFile.write(reinterpret_cast<const char*>(buildEntry.storedData.data()), static_cast<std::streamsize>(buildEntry.storedData.size()));
		}
	}

	const uint64_t indexOffset = static_cast<uint64_t>(pakFile.tellp());
	for (const PakBuildEntry& buildEntry : buildEntries)
	{
		PakEntryRecord entryRecord{};
		entryRecord.primaryPathHash = Obfuscate64(buildEntry.key.primary, headerNonce ^ 0x10ull);
		entryRecord.secondaryPathHash = Obfuscate64(buildEntry.key.secondary, headerNonce ^ 0x20ull);
		entryRecord.dataOffset = Obfuscate64(buildEntry.dataOffset, headerNonce ^ 0x30ull);
		entryRecord.storedSize = Obfuscate64(buildEntry.storedSize, headerNonce ^ 0x40ull);
		entryRecord.originalSize = Obfuscate64(buildEntry.originalSize, headerNonce ^ 0x50ull);
		entryRecord.dataCrc32 = Obfuscate64(buildEntry.dataCrc32, headerNonce ^ 0x60ull);
		entryRecord.fileNonce = Obfuscate64(buildEntry.fileNonce, headerNonce ^ 0x70ull);
		entryRecord.compressionMethod = Obfuscate64(buildEntry.compressionMethod, headerNonce ^ 0x80ull);
		pakFile.write(reinterpret_cast<const char*>(&entryRecord), sizeof(PakEntryRecord));
	}

	const uint64_t indexSize = static_cast<uint64_t>(pakFile.tellp()) - indexOffset;
	header.mountPointLength = Obfuscate64(static_cast<uint64_t>(normalizedMountPoint.size()), headerNonce ^ kHeaderMountPointKey);
	header.entryCount = Obfuscate64(static_cast<uint64_t>(buildEntries.size()), headerNonce ^ kHeaderEntryCountKey);
	header.indexOffset = Obfuscate64(indexOffset, headerNonce ^ kHeaderIndexOffsetKey);
	header.indexSize = Obfuscate64(indexSize, headerNonce ^ kHeaderIndexSizeKey);

	pakFile.seekp(0, std::ios::beg);
	pakFile.write(reinterpret_cast<const char*>(&header), sizeof(PakHeader));
	return pakFile.good();
}

bool DataEncryption::MountPak(const std::string& pakPath)
{
	const std::string normalizedPakPath = NormalizePath(pakPath);
	std::ifstream pakFile(normalizedPakPath, std::ios::binary);
	if (!pakFile.is_open())
	{
		return false;
	}

	PakHeader header{};
	pakFile.read(reinterpret_cast<char*>(&header), sizeof(PakHeader));
	if (!pakFile.good() || header.magic != kPakMagic || header.version != kPakVersion)
	{
		return false;
	}

	const uint64_t headerNonce = header.headerNonce;
	const uint64_t mountPointLength = Deobfuscate64(header.mountPointLength, headerNonce ^ kHeaderMountPointKey);
	const uint64_t entryCount = Deobfuscate64(header.entryCount, headerNonce ^ kHeaderEntryCountKey);
	const uint64_t indexOffset = Deobfuscate64(header.indexOffset, headerNonce ^ kHeaderIndexOffsetKey);
	const uint64_t indexSize = Deobfuscate64(header.indexSize, headerNonce ^ kHeaderIndexSizeKey);

	if (mountPointLength == 0u || indexSize < entryCount * sizeof(PakEntryRecord))
	{
		return false;
	}

	std::string mountPoint(static_cast<size_t>(mountPointLength), '\0');
	pakFile.read(mountPoint.data(), static_cast<std::streamsize>(mountPoint.size()));
	if (!pakFile.good())
	{
		return false;
	}

	MountedPak mountedPak;
	mountedPak.pakPath = normalizedPakPath;
	mountedPak.mountPoint = NormalizeMountPoint(mountPoint);
	mountedPak.entries.reserve(static_cast<size_t>(entryCount));

	pakFile.seekg(static_cast<std::streamoff>(indexOffset), std::ios::beg);
	if (!pakFile.good())
	{
		return false;
	}

	for (uint64_t entryIndex = 0u; entryIndex < entryCount; ++entryIndex)
	{
		PakEntryRecord entryRecord{};
		pakFile.read(reinterpret_cast<char*>(&entryRecord), sizeof(PakEntryRecord));
		if (!pakFile.good())
		{
			return false;
		}

		MountedPakEntry mountedPakEntry{};
		mountedPakEntry.key.primary = Deobfuscate64(entryRecord.primaryPathHash, headerNonce ^ 0x10ull);
		mountedPakEntry.key.secondary = Deobfuscate64(entryRecord.secondaryPathHash, headerNonce ^ 0x20ull);
		mountedPakEntry.dataOffset = Deobfuscate64(entryRecord.dataOffset, headerNonce ^ 0x30ull);
		mountedPakEntry.storedSize = Deobfuscate64(entryRecord.storedSize, headerNonce ^ 0x40ull);
		mountedPakEntry.originalSize = Deobfuscate64(entryRecord.originalSize, headerNonce ^ 0x50ull);
		mountedPakEntry.dataCrc32 = static_cast<uint32_t>(Deobfuscate64(entryRecord.dataCrc32, headerNonce ^ 0x60ull));
		mountedPakEntry.fileNonce = Deobfuscate64(entryRecord.fileNonce, headerNonce ^ 0x70ull);
		mountedPakEntry.compressionMethod = static_cast<uint32_t>(Deobfuscate64(entryRecord.compressionMethod, headerNonce ^ 0x80ull));

		mountedPak.entryLookup[mountedPakEntry.key] = mountedPak.entries.size();
		mountedPak.entries.emplace_back(mountedPakEntry);
	}

	std::lock_guard<std::mutex> lock(mountedPakMutex);
	const auto existingPakIterator = std::find_if(mountedPaks.begin(), mountedPaks.end(), [&normalizedPakPath](const MountedPak& existingPak)
		{
			return existingPak.pakPath == normalizedPakPath;
		});
	if (existingPakIterator == mountedPaks.end())
	{
		mountedPaks.emplace_back(std::move(mountedPak));
	}

	return true;
}

void DataEncryption::ResetMountedPaks()
{
	std::lock_guard<std::mutex> lock(mountedPakMutex);
	mountedPaks.clear();
	publishedMountScanAttempted = false;
}

bool DataEncryption::ReadBinaryFile(const std::string& path, std::vector<uint8_t>& outData)
{
	outData.clear();
	const std::string normalizedPath = NormalizePath(path);

	if (IsPublishedBuild())
	{
		// Runtime-saved files such as GameConfig.ini should override the packaged defaults.
		if (ReadFileBytesFromDisk(normalizedPath, outData))
		{
			return true;
		}

		EnsurePublishedPaksMounted();

		bool pathMatchesMountedMountPoint = false;
		if (TryReadFromMountedPaks(normalizedPath, outData, pathMatchesMountedMountPoint))
		{
			return true;
		}

		if (pathMatchesMountedMountPoint)
		{
			return false;
		}
	}

	return ReadFileBytesFromDisk(normalizedPath, outData);
}

bool DataEncryption::ReadTextFile(const std::string& path, std::string& outText)
{
	std::vector<uint8_t> fileBytes;
	if (!ReadBinaryFile(path, fileBytes))
	{
		outText.clear();
		return false;
	}

	outText.assign(reinterpret_cast<const char*>(fileBytes.data()), fileBytes.size());
	return true;
}

bool DataEncryption::FileExists(const std::string& path)
{
	const std::string normalizedPath = NormalizePath(path);
	std::error_code errorCode;
	if (std::filesystem::exists(std::filesystem::path(normalizedPath), errorCode))
	{
		return true;
	}

	if (IsPublishedBuild())
	{
		EnsurePublishedPaksMounted();

		std::vector<uint8_t> unusedBuffer;
		bool pathMatchesMountedMountPoint = false;
		if (TryReadFromMountedPaks(normalizedPath, unusedBuffer, pathMatchesMountedMountPoint))
		{
			return true;
		}

		if (pathMatchesMountedMountPoint)
		{
			return false;
		}
	}

	return false;
}

void DataEncryption::EnsurePublishedPaksMounted()
{
	if (!IsPublishedBuild())
	{
		return;
	}

	{
		std::lock_guard<std::mutex> lock(mountedPakMutex);
		if (publishedMountScanAttempted)
		{
			return;
		}

		publishedMountScanAttempted = true;
	}

	std::unordered_set<std::string> discoveredPakPaths;
	const std::filesystem::path searchDirectories[] =
	{
		std::filesystem::current_path(),
		std::filesystem::current_path() / "Paks"
	};

	for (const std::filesystem::path& searchDirectory : searchDirectories)
	{
		std::error_code errorCode;
		if (!std::filesystem::exists(searchDirectory, errorCode) || !std::filesystem::is_directory(searchDirectory, errorCode))
		{
			continue;
		}

		for (const auto& directoryEntry : std::filesystem::directory_iterator(searchDirectory, std::filesystem::directory_options::skip_permission_denied))
		{
			if (!directoryEntry.is_regular_file())
			{
				continue;
			}

			std::string extension = directoryEntry.path().extension().generic_string();
			std::transform(extension.begin(), extension.end(), extension.begin(), [](const unsigned char character)
				{
					return static_cast<char>(std::tolower(character));
				});

			if (extension != ".pak")
			{
				continue;
			}

			discoveredPakPaths.insert(NormalizePath(directoryEntry.path().generic_string()));
		}
	}

	for (const std::string& discoveredPakPath : discoveredPakPaths)
	{
		MountPak(discoveredPakPath);
	}
}

bool DataEncryption::IsPublishedBuild()
{
#ifdef GOKNAR_PUBLISHED_BUILD
	return true;
#else
	return false;
#endif
}

std::string DataEncryption::NormalizePath(const std::string& path)
{
	if (path.empty())
	{
		return "";
	}

	std::string normalizedPath = std::filesystem::path(path).lexically_normal().generic_string();
	while (normalizedPath.find("//") != std::string::npos)
	{
		normalizedPath.replace(normalizedPath.find("//"), 2, "/");
	}

	if (normalizedPath.rfind("./", 0) == 0)
	{
		normalizedPath = normalizedPath.substr(2);
	}

	return normalizedPath;
}
