#include "pch.h"

#include "Texture.h"

#include "glad/glad.h"

#ifdef GOKNAR_PLATFORM_WINDOWS
#include <GL/GLU.h>
#elif defined(GOKNAR_PLATFORM_UNIX)
#include <GL/gl.h>
#endif

#include "Goknar/Contents/Image.h"
#include "Goknar/IO/IOManager.h"
#include "Goknar/Log.h"
#include "Goknar/Renderer/Shader.h"

Texture::Texture()
{
	GUID_ = ObjectIDManager::GetInstance()->GetAndIncreaseTextureGUID();
	name_ = std::string("texture" + std::to_string(GUID_));
}

Texture::Texture(std::string imagePath) : Texture()
{
	imagePath_ = CONTENT_DIR + imagePath;
}

Texture::Texture(Image* image) :
	Texture()
{
	buffer_ = image->GetBuffer();
	width_ = image->GetWidth();
	height_ = image->GetHeight();
	channels_ = image->GetChannels();
	textureUsage_ = image->GetTextureUsage();
	textureWrappingR_ = image->GetTextureWrappingR();
	textureWrappingT_ = image->GetTextureWrappingT();
	textureWrappingS_ = image->GetTextureWrappingS();

	const std::string& imageName = image->GetName();
	if (!imageName.empty())
	{
		name_ = imageName;
	}
}

Texture::~Texture()
{
	glDeleteTextures(1, &rendererTextureId_);
	delete[] buffer_;
}

void Texture::ReadFromFrameBuffer(GEuint framebuffer)
{
	if (channels_ == 0)
	{
		return;
	}
	delete[] buffer_;
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	buffer_ = new unsigned char[width_ * height_ * channels_];
	glReadPixels(0, 0, width_, height_, (int)textureFormat_, GL_UNSIGNED_BYTE, (GLvoid*)buffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Texture::Save(std::string path)
{
	if (!buffer_)
	{
		GOKNAR_CORE_WARN("NULL Texture buffer was tried to be saved.");
		return;
	}
	IOManager::WritePng(path.c_str(), width_, height_, channels_, buffer_);
}

void Texture::PreInit()
{
	// Skip if already initialized
	if (isInitialized_)
	{
		return;
	}

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
	else if (!buffer_ && !LoadTextureImage())
	{
		GOKNAR_CORE_ERROR("Texture file at {0} could not be found!", imagePath_);
		return;
	}

	if (channels_ == 4)
	{
		if(textureFormat_ != TextureFormat::RGBA)
		{
			textureFormat_ = TextureFormat::RGBA;
		}

		if(	textureInternalFormat_ != TextureInternalFormat::RGBA &&
			textureInternalFormat_ != TextureInternalFormat::RGBA16F &&
			textureInternalFormat_ != TextureInternalFormat::RGBA32F)
		{
			textureInternalFormat_ = TextureInternalFormat::RGBA;
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	int textureBindTargetInt = (int)textureBindTarget_;

	glGenTextures(1, &rendererTextureId_);
	glActiveTexture(GL_TEXTURE0 + rendererTextureId_);
	glBindTexture(textureBindTargetInt, rendererTextureId_);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D((int)textureImageTarget_, 0, (int)textureInternalFormat_, width_, height_, 0, (int)textureFormat_, (int)textureType_, buffer_);

	if (textureImageTarget_ == TextureImageTarget::TEXTURE_CUBE_MAP_POSITIVE_X)
	{
		for (int i = 1; i < 6; ++i)
		{
			glTexImage2D((int)textureImageTarget_ + i, 0, (int)textureInternalFormat_, width_, height_, 0, (int)textureFormat_, (int)textureType_, buffer_);
		}
	}

	glTexParameteri(textureBindTargetInt, GL_TEXTURE_COMPARE_MODE, (int)textureCompareMode_);

	if (textureCompareMode_ == TextureCompareMode::COMPARE_REF_TO_TEXTURE)
	{
		glTexParameteri(textureBindTargetInt, GL_TEXTURE_COMPARE_FUNC, (int)textureCompareFunc_);
	}

	glTexParameteri(textureBindTargetInt, GL_TEXTURE_MIN_FILTER, (int)minFilter_);
	glTexParameteri(textureBindTargetInt, GL_TEXTURE_MAG_FILTER, (int)magFilter_);

	glTexParameteri(textureBindTargetInt, GL_TEXTURE_WRAP_S, (int)textureWrappingS_);
	glTexParameteri(textureBindTargetInt, GL_TEXTURE_WRAP_T, (int)textureWrappingT_);
	glTexParameteri(textureBindTargetInt, GL_TEXTURE_WRAP_R, (int)textureWrappingR_);

	if (generateMipmap_ && textureFormat_ != TextureFormat::DEPTH && textureFormat_ != TextureFormat::DEPTH_STENCIL)
	{
		glGenerateMipmap(textureBindTargetInt);
	}
	
	glBindTexture(textureBindTargetInt, 0);
	
	EXIT_ON_GL_ERROR("Texture::Init");

	delete[] buffer_;
	buffer_ = nullptr;
}

void Texture::Init()
{
}

void Texture::PostInit()
{
	isInitialized_ = true;
}

void Texture::Bind(const Shader* shader) const
{
	if (shader != nullptr)
	{
		shader->SetInt(name_.c_str(), rendererTextureId_);
	}

	glActiveTexture(GL_TEXTURE0 + rendererTextureId_);
	glBindTexture((int)textureBindTarget_, rendererTextureId_);
	EXIT_ON_GL_ERROR("Texture::Bind");
}

void Texture::Unbind()
{
	glBindTexture((int)textureBindTarget_, 0);
}

bool Texture::LoadTextureImage()
{
	return IOManager::ReadImage(imagePath_.c_str(), width_, height_, channels_, &buffer_);
}

void Texture::UpdateSizeOnGPU()
{
	glTexImage2D((int)textureImageTarget_, 0, (int)textureInternalFormat_, width_, height_, 0, (int)textureFormat_, (int)textureType_, buffer_);

	if (textureImageTarget_ == TextureImageTarget::TEXTURE_CUBE_MAP_POSITIVE_X)
	{
		for (int i = 1; i < 6; ++i)
		{
			glTexImage2D((int)textureImageTarget_ + i, 0, (int)textureInternalFormat_, width_, height_, 0, (int)textureFormat_, (int)textureType_, buffer_);
		}
	}
}