#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "Core.h"
#include "Contents/Content.h"
#include "Renderer/Texture.h"

class GOKNAR_API Image : public Content
{
public:
	Image();
	Image(const std::string& path);
	Image(const std::string& path, int width, int height, int channels, unsigned char* buffer);
	virtual ~Image() = default;

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	const unsigned char* GetBuffer() const
	{
		return buffer_;
	}

	void SetTextureUsage(TextureUsage textureUsage)
	{
		textureUsage_ = textureUsage;
	}

	TextureUsage GetTextureUsage() const
	{
		return textureUsage_;
	}

	Texture* GetGeneratedTexture() const
	{
		return generatedTexture_;
	}

	int GetWidth() const
	{
		return width_;
	}

	int GetHeight() const
	{
		return height_;
	}

	int GetChannels() const
	{
		return channels_;
	}

private:
	Texture* generatedTexture_;

	unsigned char* buffer_;

	int width_;
	int height_;
	int channels_;

	TextureUsage textureUsage_;
};

#endif 