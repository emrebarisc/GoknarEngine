#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "Goknar/Core.h"
#include "Goknar/Managers/ObjectIDManager.h"
#include "Types.h"

#include <string>

class Shader;

enum class TextureWrapping
{
	REPEAT = GL_REPEAT,
	MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
	CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
	CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER
};

class GOKNAR_API Texture
{
public:
	Texture();
	Texture(std::string imagePath);

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

	int objectId_;
	int width_;
	int height_;
	int channels_;
};

#endif