#ifndef __DATAENCRYPTION_H__
#define __DATAENCRYPTION_H__

#include "Goknar/Core.h"

#include <cstdint>
#include <string>
#include <vector>

class GOKNAR_API DataEncryption
{
public:
	static bool CreatePakFromDirectory(const std::string& sourceDirectory, const std::string& pakPath, const std::string& mountPoint);
	static bool MountPak(const std::string& pakPath);
	static void ResetMountedPaks();

	static bool ReadBinaryFile(const std::string& path, std::vector<uint8_t>& outData);
	static bool ReadTextFile(const std::string& path, std::string& outText);
	static bool FileExists(const std::string& path);

	static void EnsurePublishedPaksMounted();
	static bool IsPublishedBuild();
	static std::string NormalizePath(const std::string& path);
};

#endif
