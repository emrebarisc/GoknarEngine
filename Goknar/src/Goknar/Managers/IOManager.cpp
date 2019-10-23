#include "pch.h"

#include "IOManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

IOManager::IOManager()
{
}

IOManager::~IOManager()
{
}

bool IOManager::GetFileRawText(const char* filePath, char* buffer)
{
	FILE* file = fopen(filePath, "rb");

	if(!file)
	{
		return false;
	}

	fseek(file, 0, SEEK_END);
	long fileLength = ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer = (char*)malloc(fileLength * sizeof(char));

	if (buffer)
	{
		fread(buffer, 1, fileLength, file);
	}

	fclose(file);

	return true;
}

bool IOManager::ReadImage(const char* filePath, int& width, int& height, int& channels, unsigned char* rawDataBuffer)
{
	rawDataBuffer = stbi_load(filePath, &width, &height, &channels, 0);
	return rawDataBuffer != nullptr;
}
