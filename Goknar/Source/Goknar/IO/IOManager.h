#ifndef __IOMANAGER_H__
#define __IOMANAGER_H__

#include "Goknar/Core.h"

class Image;

template<typename T>
class Mesh;

class GOKNAR_API IOManager
{
public:
	IOManager();
	~IOManager();

	static bool ReadFile(const char* filePath, std::string& rawTextBuffer);
	static bool WriteFile(const char* filePath, const char* rawTextBuffer);

	static Image* LoadImage(const std::string& filePath);
	static bool ReadImage(const char* filePath, int& width, int& height, int& channels, const unsigned char** rawDataBuffer);
	static bool WritePng(const char* filePath, int width, int height, int channels, const unsigned char* rawDataBuffer);
	static bool WritePpm(const char* filePath, int width, int height, const unsigned char* rawDataBuffer);

	template<typename T>
	static T* LoadModel(const std::string& path);

protected:

private:
};

#endif