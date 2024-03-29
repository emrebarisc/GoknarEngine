#include "pch.h"

#include "IOManager.h"

#include <iostream>

#include "Log.h"
#include "ModelLoader.h"
#include "Contents/Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

IOManager::IOManager()
{
    stbi_set_flip_vertically_on_load(true);
}

IOManager::~IOManager()
{
}

bool IOManager::ReadFile(const char* filePath, std::string& buffer)
{
    std::fstream file;

    file.open(filePath, std::ios::in);

    if (file.is_open()) {
        std::string currentLine;

        while (getline(file, currentLine)) {
            buffer += currentLine;
            if (!file.eof())
                buffer += "\n";
        }

        file.close();
    }
    else
    {
        GOKNAR_CORE_ERROR("File is not found on {}.", filePath);
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
    int width;
    int height;
    int channels;
    unsigned char* buffer = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

    if (buffer == nullptr)
    {
        return nullptr;
    }

    Image* image = new Image(filePath, width, height, channels, buffer);
    return image;
}

bool IOManager::ReadImage(const char* filePath, int& width, int& height, int& channels, const unsigned char** rawDataBuffer)
{
	*rawDataBuffer = stbi_load(filePath, &width, &height, &channels, 0);
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

StaticMesh* IOManager::LoadPlyFile(const std::string& path)
{
    return ModelLoader::LoadPlyFile(path);
}

StaticMesh* IOManager::LoadModel(const std::string& path)
{
    return ModelLoader::LoadModel(path);
}
