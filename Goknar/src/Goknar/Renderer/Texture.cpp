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
	textureTarget_(TextureTarget::TEXTURE_2D),
	textureWrappingS_(TextureWrapping::REPEAT),
	textureWrappingT_(TextureWrapping::REPEAT),
	textureWrappingR_(TextureWrapping::REPEAT),
	minFilter_(TextureMinFilter::LINEAR),
	magFilter_(TextureMagFilter::LINEAR),
	textureFormat_(TextureFormat::RGB),
	textureType_(TextureType::UNSIGNED_BYTE),
	textureDataType_(TextureDataType::STATIC)
{
	objectId_ = ObjectIDManager::GetInstance()->GetAndIncreaseTextureID();
	name_ = std::string("texture" + std::to_string(objectId_));
}

Texture::Texture(std::string imagePath) : Texture()
{
	imagePath_ = imagePath;
}

Texture::~Texture()
{
	delete[] buffer_;
	glDeleteTextures(1, &rendererTextureId_);
}

void Texture::ReadFromFramebuffer(GEuint framebuffer)
{
	if (channels_ == 0)
	{
		return;
	}
	delete buffer_;
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	buffer_ = new unsigned char[width_ * height_ * channels_];
	glReadPixels(0, 0, width_, height_, (int)textureFormat_, GL_UNSIGNED_BYTE, (GLvoid*)buffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Texture::Save(std::string path)
{
	IOManager::WritePng(path.c_str(), width_, height_, channels_, buffer_);
}

void Texture::Init()
{
	if (textureDataType_ == TextureDataType::DYNAMIC)
	{
		if (textureFormat_ == TextureFormat::DEPTH || textureFormat_ == TextureFormat::RED)
		{
			channels_ = 1;
		}
		else if (textureFormat_ == TextureFormat::RG)
		{
			channels_ = 2;
		}
		else if (textureFormat_ == TextureFormat::RGB)
		{
			channels_ = 3;
		}
		else if (textureFormat_ == TextureFormat::RGBA)
		{
			channels_ = 4;
		}
	}
	else if (!LoadTextureImage())
	{
		GOKNAR_CORE_ERROR("Texture file {0} could not be found!", imagePath_);
		return;
	}

	if (channels_ == 4)
	{
		textureFormat_ = TextureFormat::RGBA;
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &rendererTextureId_);
	glActiveTexture(GL_TEXTURE0 + rendererTextureId_);
	glBindTexture((int)textureTarget_, rendererTextureId_);

	glTexImage2D((int)textureTarget_, 0, (int)textureFormat_, width_, height_, 0, (int)textureFormat_, (int)textureType_, buffer_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glTexParameteri((int)textureTarget_, GL_TEXTURE_MIN_FILTER, (int)minFilter_);
	glTexParameteri((int)textureTarget_, GL_TEXTURE_MAG_FILTER, (int)magFilter_);

	glTexParameteri((int)textureTarget_, GL_TEXTURE_WRAP_S, (int)textureWrappingS_);
	glTexParameteri((int)textureTarget_, GL_TEXTURE_WRAP_T, (int)textureWrappingT_);
	glTexParameteri((int)textureTarget_, GL_TEXTURE_WRAP_R, (int)textureWrappingR_);

	if (textureFormat_ != TextureFormat::DEPTH && textureFormat_ != TextureFormat::DEPTH_STENCIL)
	{
		glGenerateMipmap((int)textureTarget_);
	}
	
	glBindTexture((int)textureTarget_, 0);
}

void Texture::Bind(Shader* shader) const
{
	if (shader != nullptr)
	{
		shader->SetInt(name_.c_str(), rendererTextureId_);
	}

	glActiveTexture(GL_TEXTURE0 + rendererTextureId_);
	glBindTexture((int)textureTarget_, rendererTextureId_);
}

void Texture::Unbind()
{
	glBindTexture((int)textureTarget_, 0);
}

bool Texture::LoadTextureImage()
{
	return IOManager::ReadImage(imagePath_.c_str(), width_, height_, channels_, &buffer_);
}