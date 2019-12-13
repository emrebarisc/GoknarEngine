#include "pch.h"

#include "Goknar/Log.h"
#include "Texture.h"
#include "Goknar/Managers/IOManager.h"

#include "glad/glad.h"

#include <gl/GLU.h>

void Texture::Init()
{
	if (!LoadTextureImage())
	{
		GOKNAR_CORE_ERROR("Texture file could not be found!");
		return;
	}

	glGenTextures(1, &textureId_);
	glBindTexture(GL_TEXTURE_2D, textureId_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, channels_ == 3 ? GL_RGB : GL_RGBA, width_, height_, 0, channels_ == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, buffer_);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, textureId_);
}

bool Texture::LoadTextureImage()
{
	return IOManager::ReadImage(imagePath_, width_, height_, channels_, &buffer_);
}