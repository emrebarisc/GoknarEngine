#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "Types.h"

class Texture
{
public:
	Texture() :
		imagePath_(""),
		buffer_(nullptr),
		textureId_(-1),
		width_(0),
		height_(0),
		channels_(0)
	{}

	~Texture() 
	{
		delete[] buffer_;
	}

	void SetTextureImagePath(const char* imagePath)
	{
		imagePath_ = imagePath;
	}

	GEuint GetTextureId() const
	{
		return textureId_;
	}

	bool LoadTextureImage();

	void Init();
	void Bind() const;

protected:

private:
	const char* imagePath_;
	unsigned char* buffer_;
	GEuint textureId_;

	int width_;
	int height_;
	int channels_;
};

#endif