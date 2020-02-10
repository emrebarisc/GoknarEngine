#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "Goknar/Managers/ObjectIDManager.h"
#include "Types.h"

class Shader;

enum class TextureWrapping
{
	REPEAT = GL_REPEAT,
	MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
	CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
	CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER
};

enum class TextureMinFilter
{
	NEAREST = GL_NEAREST,
	LINEAR = GL_LINEAR,
	NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
	LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR
};

enum class TextureMagFilter
{
	NEAREST = GL_NEAREST,
	LINEAR = GL_LINEAR
};

class Texture
{
public:
	Texture() :
		imagePath_(""),
		buffer_(nullptr),
		width_(0),
		height_(0),
		channels_(0),
		rendererTextureId_(-1),
		textureWrapping_(TextureWrapping::CLAMP_TO_BORDER),
		minFilter_(TextureMinFilter::LINEAR),
		magFilter_(TextureMagFilter::LINEAR)
	{
		objectId_ = ObjectIDManager::GetInstance()->GetAndIncreaseTextureID();
		name_ = std::string("texture" + std::to_string(objectId_));
	}

	~Texture() 
	{
		delete[] buffer_;
	}

	void SetTextureImagePath(const std::string& imagePath)
	{
		imagePath_ = imagePath;
	}

	GEuint GetTextureObjectId() const
	{
		return objectId_;
	}

	const std::string& GetName() const
	{
		return name_;
	}

	void SetName(const std::string& name)
	{
		name_ = name;
	}

	TextureWrapping GetTextureWrapping() const
	{
		return textureWrapping_;
	}

	void SetTextureWrapping(TextureWrapping textureWrapping)
	{
		textureWrapping_ = textureWrapping;
	}

	TextureMinFilter GetTextureMinFilter() const
	{
		return minFilter_;
	}

	void SetTextureMinFilter(TextureMinFilter minFilter)
	{
		minFilter_ = minFilter;
	}

	TextureMagFilter GetTextureMagFilter() const
	{
		return magFilter_;
	}

	void SetTextureMagFilter(TextureMagFilter magFilter)
	{
		magFilter_ = magFilter;
	}

	bool LoadTextureImage();

	void Init();
	void Bind(Shader* shader) const;

protected:

private:
	std::string name_;
	std::string imagePath_;
	unsigned char* buffer_;
	GEuint rendererTextureId_;

	TextureWrapping textureWrapping_;
	TextureMinFilter minFilter_;
	TextureMagFilter magFilter_;

	int objectId_;
	int width_;
	int height_;
	int channels_;
};

#endif