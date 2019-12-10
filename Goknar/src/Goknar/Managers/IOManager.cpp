#include "pch.h"

#include "IOManager.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

IOManager::IOManager()
{
}

IOManager::~IOManager()
{
}

bool IOManager::ReadFile(const char* filePath, char** buffer)
{
	FILE* file = fopen(filePath, "r");

	if(!file)
	{
		return false;
	}

	fseek(file, 0, SEEK_END);
	long fileLength = ftell(file);
	fseek(file, 0, SEEK_SET);

	fileLength++;

	*buffer = (char*)malloc(fileLength * sizeof(char));

	if (*buffer)
	{
		fread((void*)*buffer, 1, fileLength, file);
	}

	(*buffer)[fileLength - 1] = '\0';

	fclose(file);
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

bool IOManager::ReadImage(const char* filePath, int& width, int& height, int& channels, unsigned char** rawDataBuffer)
{
	*rawDataBuffer = stbi_load(filePath, &width, &height, &channels, 0);
	return *rawDataBuffer != nullptr;
}

bool IOManager::WritePng(const char* filePath, int& width, int& height, int& channels, const unsigned char* rawDataBuffer)
{
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

                if (i == width - 1 && c == 2)
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