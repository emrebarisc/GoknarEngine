#include "pch.h"

#include "IOManager.h"

#include <iostream>

#include "Contents/Content.h"
#include "Log.h"
#include "ModelLoader.h"
#include "Contents/Image.h"
#include "Goknar/Data/DataEncryption.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace
{
	unsigned char* LoadImageBufferFromBytes(const std::vector<uint8_t>& imageBytes, int& width, int& height, int& channels)
	{
		if (imageBytes.empty())
		{
			return nullptr;
		}

		stbi_uc* stbBuffer = stbi_load_from_memory(
			imageBytes.data(),
			static_cast<int>(imageBytes.size()),
			&width,
			&height,
			&channels,
			0);
		if (stbBuffer == nullptr)
		{
			return nullptr;
		}

		const size_t bufferSize = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels);
		unsigned char* ownedBuffer = new unsigned char[bufferSize];
		std::memcpy(ownedBuffer, stbBuffer, bufferSize);
		stbi_image_free(stbBuffer);
		return ownedBuffer;
	}
}

IOManager::IOManager()
{
    stbi_set_flip_vertically_on_load(true);
}

IOManager::~IOManager()
{
}

bool IOManager::ReadFile(const char* filePath, std::string& buffer)
{
	buffer.clear();
	if (!DataEncryption::ReadTextFile(filePath, buffer))
	{
		GOKNAR_CORE_ERROR("File is not found on %s.", filePath);
		return false;
	}

	return true;
}

bool IOManager::WriteFile(const char* filePath, const char* rawTextBuffer)
{
	FILE* file;
	file = fopen(filePath, "w");
	if (!file)
	{ 
		return false;
	}
	fprintf(file, "%s", rawTextBuffer);
	fclose(file);

	return true;
}

Image* IOManager::LoadImage(const std::string& filePath)
{
	std::vector<uint8_t> imageBytes;
	if (!DataEncryption::ReadBinaryFile(filePath, imageBytes))
	{
		return nullptr;
	}

	int width = 0;
	int height = 0;
	int channels = 0;
	unsigned char* buffer = LoadImageBufferFromBytes(imageBytes, width, height, channels);
	if (buffer == nullptr)
	{
		return nullptr;
	}

	return new Image(filePath, width, height, channels, buffer);
}

bool IOManager::ReadImage(const char* filePath, int& width, int& height, int& channels, const unsigned char** rawDataBuffer)
{
	std::vector<uint8_t> imageBytes;
	if (!DataEncryption::ReadBinaryFile(filePath, imageBytes))
	{
		*rawDataBuffer = nullptr;
		return false;
	}

	*rawDataBuffer = LoadImageBufferFromBytes(imageBytes, width, height, channels);
	return *rawDataBuffer != nullptr;
}

bool IOManager::WritePng(const char* filePath, int width, int height, int channels, const unsigned char* rawDataBuffer)
{
    if (rawDataBuffer == nullptr)
    {
        return false;
    }

	return stbi_write_png(filePath, width, height, channels, rawDataBuffer, width * channels) == 1;
}

bool IOManager::WritePpm(const char* filePath, int width, int height, const unsigned char* rawDataBuffer)
{
    FILE* outfile;

    std::string ppmFileName(filePath);
    ppmFileName.append(".ppm");

    if ((outfile = fopen(ppmFileName.c_str(), "w")) == NULL)
    {
        std::cerr << "Error: The ppm file could be opened for writing." << std::endl;
        return false;
    }

    (void)fprintf(outfile, "P3\n%d %d\n255\n", width, height);

    unsigned char color;
    for (size_t j = 0, idx = 0; j < height; ++j)
    {
        for (size_t i = 0; i < width; ++i)
        {
            for (size_t c = 0; c < 3; ++c, ++idx)
            {
                color = (unsigned char)(int)rawDataBuffer[idx];

                if (i == (width - 1) && c == 2)
                {
                    (void)fprintf(outfile, "%d", color);
                }
                else
                {
                    (void)fprintf(outfile, "%d ", color);
                }
            }
        }

        (void)fprintf(outfile, "\n");
    }

    (void)fclose(outfile);

    return true;
}

Content* IOManager::LoadModel(const std::string& path)
{
    return ModelLoader::LoadModel(path);
}
