#include "pch.h"

#include "Texture.h"

#include "glad/glad.h"
#include <gl/GLU.h>

#include "Goknar/IO/IOManager.h"
#include "Goknar/Log.h"
#include "Goknar/Renderer/Shader.h"

Texture::Texture() :
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

Texture::Texture(std::string imagePath) : Texture()
{
	imagePath_ = imagePath;
}

void Texture::Init()
{
	if (!LoadTextureImage())
	{
		GOKNAR_CORE_ERROR("Texture file could not be found!");
		return;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &rendererTextureId_);
	glActiveTexture(GL_TEXTURE0 + rendererTextureId_);
	glBindTexture(GL_TEXTURE_2D, rendererTextureId_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)textureWrapping_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)textureWrapping_);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)minFilter_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)magFilter_);

	glTexImage2D(GL_TEXTURE_2D, 0, channels_ == 3 ? GL_RGB : GL_RGBA, width_, height_, 0, channels_ == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, buffer_);

	if (channels_ == 4)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glGenerateMipmap(GL_TEXTURE_2D);
		
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Bind(Shader* shader) const
{
	shader->SetInt(name_.c_str(), rendererTextureId_);
	glActiveTexture(GL_TEXTURE0 + rendererTextureId_);
	glBindTexture(GL_TEXTURE_2D, rendererTextureId_);
}

bool Texture::LoadTextureImage()
{
	return IOManager::ReadImage(imagePath_.c_str(), width_, height_, channels_, &buffer_);
}