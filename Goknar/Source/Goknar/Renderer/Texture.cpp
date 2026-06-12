#include "pch.h"

#include "Texture.h"

#include "Goknar/Contents/Image.h"
#include "Goknar/Engine.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/IO/IOManager.h"
#include "Goknar/Log.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Framebuffer.h"

#include <cctype>
#include <cstring>

Texture::Texture()
{
	GUID_ = ObjectIDManager::GetInstance()->GetAndIncreaseTextureGUID();
	name_ = std::string("texture" + std::to_string(GUID_));
}

Texture::Texture(std::string imagePath) : Texture()
{
	imagePath_ = ContentDir + imagePath;
}

Texture::Texture(Image* image) :
	Texture()
{
	width_ = image->GetWidth();
	height_ = image->GetHeight();
	channels_ = image->GetChannels();
	textureUsage_ = image->GetTextureUsage();
	textureWrappingR_ = image->GetTextureWrappingR();
	textureWrappingT_ = image->GetTextureWrappingT();
	textureWrappingS_ = image->GetTextureWrappingS();
	imagePath_ = image->GetPath();

	const unsigned char* imageBuffer = image->GetBuffer();
	if (imageBuffer && width_ > 0 && height_ > 0 && channels_ > 0)
	{
		const size_t bufferSize = static_cast<size_t>(width_) * static_cast<size_t>(height_) * static_cast<size_t>(channels_);
		unsigned char* copiedBuffer = new unsigned char[bufferSize];
		std::memcpy(copiedBuffer, imageBuffer, bufferSize);
		buffer_ = copiedBuffer;
	}

	const std::string& imageName = image->GetName();
	if (!imageName.empty())
	{
		name_ = imageName;
	}
}

Texture::~Texture()
{
	if (atlasProxySourceImage_)
	{
		atlasProxySourceImage_->UnregisterTextureAtlasProxy(this);
	}

	engine->GetGraphicsAPI()->DeleteTexture(rendererTextureId_);
	delete[] buffer_;
}

void Texture::ReadFromFrameBuffer(GEuint framebuffer)
{
	if (channels_ == 0)
	{
		return;
	}
	delete[] buffer_;
	engine->GetGraphicsAPI()->BindFrameBuffer(FrameBufferBindTarget::FRAMEBUFFER, framebuffer);
	buffer_ = new unsigned char[width_ * height_ * channels_];
	engine->GetGraphicsAPI()->ReadPixels(0, 0, width_, height_, textureFormat_, TextureType::UNSIGNED_BYTE, const_cast<unsigned char*>(buffer_));
	engine->GetGraphicsAPI()->BindFrameBuffer(FrameBufferBindTarget::FRAMEBUFFER, 0);
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

void Texture::GenerateMipmap() const
{
	if (!generateMipmap_ || textureFormat_ == TextureFormat::DEPTH || textureFormat_ == TextureFormat::DEPTH_STENCIL)
	{
		return;
	}

	engine->GetGraphicsAPI()->BindTexture(textureBindTarget_, rendererTextureId_);
	engine->GetGraphicsAPI()->GenerateMipmap(textureBindTarget_);
	engine->GetGraphicsAPI()->BindTexture(textureBindTarget_, 0);
	EXIT_ON_GRAPHICS_API_ERROR("Texture::GenerateMipmap");
}

void Texture::PreInit()
{
	// Skip if already initialized or if this texture is an atlas proxy.
	// Atlas proxies are logical texture handles used by materials; they must not
	// upload their own GPU image while waiting for TextureAtlas::Build() to assign
	// the shared atlas texture.
	if (isInitialized_ || atlasTexture_ || rendererTextureId_ != 0)
	{
		return;
	}

	if (waitsForTextureAtlas_)
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

		engine->GetGraphicsAPI()->SetBlendFunction(GraphicsBlendFactor::SourceAlpha, GraphicsBlendFactor::OneMinusSourceAlpha);
	}

	rendererTextureId_ = engine->GetGraphicsAPI()->CreateTexture();
	engine->GetGraphicsAPI()->ActivateTextureUnit(rendererTextureId_);
	engine->GetGraphicsAPI()->BindTexture(textureBindTarget_, rendererTextureId_);

	engine->GetGraphicsAPI()->PixelStore(GraphicsPixelStoreParameter::UnpackAlignment, 1);

	if (textureBindTarget_ == TextureBindTarget::TEXTURE_3D)
	{
		engine->GetGraphicsAPI()->SetTextureImage3D(textureBindTarget_, 0, textureInternalFormat_, width_, height_, depth_, 0, textureFormat_, textureType_, buffer_);
	}
	else
	{
		engine->GetGraphicsAPI()->SetTextureImage2D(textureImageTarget_, 0, 0, textureInternalFormat_, width_, height_, 0, textureFormat_, textureType_, buffer_);
	}

	if (textureBindTarget_ != TextureBindTarget::TEXTURE_3D && textureImageTarget_ == TextureImageTarget::TEXTURE_CUBE_MAP_POSITIVE_X)
	{
		for (int i = 1; i < 6; ++i)
		{
			engine->GetGraphicsAPI()->SetTextureImage2D(textureImageTarget_, i, 0, textureInternalFormat_, width_, height_, 0, textureFormat_, textureType_, buffer_);
		}
	}

	engine->GetGraphicsAPI()->SetTextureCompareMode(textureBindTarget_, textureCompareMode_);

	if (textureCompareMode_ == TextureCompareMode::COMPARE_REF_TO_TEXTURE)
	{
		engine->GetGraphicsAPI()->SetTextureCompareFunc(textureBindTarget_, textureCompareFunc_);
	}

	engine->GetGraphicsAPI()->SetTextureMinFilter(textureBindTarget_, minFilter_);
	engine->GetGraphicsAPI()->SetTextureMagFilter(textureBindTarget_, magFilter_);

	engine->GetGraphicsAPI()->SetTextureWrappingS(textureBindTarget_, textureWrappingS_);
	engine->GetGraphicsAPI()->SetTextureWrappingT(textureBindTarget_, textureWrappingT_);
	engine->GetGraphicsAPI()->SetTextureWrappingR(textureBindTarget_, textureWrappingR_);

	if (generateMipmap_ && textureFormat_ != TextureFormat::DEPTH && textureFormat_ != TextureFormat::DEPTH_STENCIL)
	{
		engine->GetGraphicsAPI()->GenerateMipmap(textureBindTarget_);
	}

	engine->GetGraphicsAPI()->BindTexture(textureBindTarget_, 0);

	EXIT_ON_GRAPHICS_API_ERROR("Texture::Init");

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
	const GEuint effectiveRendererTextureId = GetEffectiveRendererTextureId();

	if (shader != nullptr)
	{
		shader->SetInt(GetShaderUniformName().c_str(), static_cast<int>(effectiveRendererTextureId));

		if (atlasTexture_)
		{
			Vector4 atlasUVTransform(0.f);
			atlasUVTransform.x = atlasUScale_;
			atlasUVTransform.y = atlasVScale_;
			atlasUVTransform.z = atlasUOffset_;
			atlasUVTransform.w = atlasVOffset_;
			shader->SetVector4(GetAtlasUVTransformUniformName().c_str(), atlasUVTransform);
		}
	}

	BindToTextureUnit(effectiveRendererTextureId);
	EXIT_ON_GRAPHICS_API_ERROR("Texture::Bind");
}

void Texture::BindToTextureUnit(unsigned int textureUnit) const
{
	engine->GetGraphicsAPI()->ActivateTextureUnit(textureUnit);
	engine->GetGraphicsAPI()->BindTexture(textureBindTarget_, GetEffectiveRendererTextureId());
	EXIT_ON_GRAPHICS_API_ERROR("Texture::BindToTextureUnit");
}

void Texture::BindAsImage(unsigned int imageUnit, TextureImageAccess access) const
{
	engine->GetGraphicsAPI()->BindImageTexture(imageUnit, GetEffectiveRendererTextureId(), 0, false, 0, access, textureInternalFormat_);
	EXIT_ON_GRAPHICS_API_ERROR("Texture::BindAsImage");
}

void Texture::Unbind()
{
	engine->GetGraphicsAPI()->BindTexture(textureBindTarget_, 0);
}

bool Texture::LoadTextureImage()
{
	return IOManager::ReadImage(imagePath_.c_str(), width_, height_, channels_, &buffer_);
}

void Texture::UpdateSizeOnGPU()
{
	if (atlasTexture_)
	{
		return;
	}

	engine->GetGraphicsAPI()->ActivateTextureUnit(rendererTextureId_);
	engine->GetGraphicsAPI()->BindTexture(textureBindTarget_, rendererTextureId_);
	if (textureBindTarget_ == TextureBindTarget::TEXTURE_3D)
	{
		engine->GetGraphicsAPI()->SetTextureImage3D(textureBindTarget_, 0, textureInternalFormat_, width_, height_, depth_, 0, textureFormat_, textureType_, buffer_);
	}
	else
	{
		engine->GetGraphicsAPI()->SetTextureImage2D(textureImageTarget_, 0, 0, textureInternalFormat_, width_, height_, 0, textureFormat_, textureType_, buffer_);
	}

	if (textureBindTarget_ != TextureBindTarget::TEXTURE_3D && textureImageTarget_ == TextureImageTarget::TEXTURE_CUBE_MAP_POSITIVE_X)
	{
		for (int i = 1; i < 6; ++i)
		{
			engine->GetGraphicsAPI()->SetTextureImage2D(textureImageTarget_, i, 0, textureInternalFormat_, width_, height_, 0, textureFormat_, textureType_, buffer_);
		}
	}

	if (generateMipmap_ && textureFormat_ != TextureFormat::DEPTH && textureFormat_ != TextureFormat::DEPTH_STENCIL)
	{
		engine->GetGraphicsAPI()->GenerateMipmap(textureBindTarget_);
	}

	engine->GetGraphicsAPI()->BindTexture(textureBindTarget_, 0);
}

void Texture::SetAtlasTexture(
	Texture* atlasTexture,
	float uMin,
	float vMin,
	float uMax,
	float vMax,
	TextureAtlasCategory atlasCategory,
	int atlasIndex)
{
	atlasTexture_ = atlasTexture;
	atlasCategory_ = atlasCategory;
	atlasIndex_ = atlasTexture ? atlasIndex : -1;
	atlasUScale_ = uMax - uMin;
	atlasVScale_ = vMax - vMin;
	atlasUOffset_ = uMin;
	atlasVOffset_ = vMin;
	generateMipmap_ = false;
	waitsForTextureAtlas_ = false;

	if (atlasTexture_)
	{
		textureBindTarget_ = atlasTexture_->GetTextureBindTarget();
		textureImageTarget_ = atlasTexture_->GetTextureImageTarget();
		textureFormat_ = atlasTexture_->GetTextureFormat();
		textureInternalFormat_ = atlasTexture_->GetTextureInternalFormat();
		textureType_ = atlasTexture_->GetTextureType();
	}
}

void Texture::SetTextureAtlasProxySourceImage(Image* image)
{
	atlasProxySourceImage_ = image;
}

std::string Texture::GetShaderUniformName() const
{
	std::string fallback = std::string("texture") + std::to_string(GUID_);

	std::string result;
	result.reserve(name_.size() + fallback.size() + 1);

	for (char character : name_)
	{
		bool isValid = std::isalnum(static_cast<unsigned char>(character)) || character == '_';
		result.push_back(isValid ? character : '_');
	}

	if (result.empty() || std::isdigit(static_cast<unsigned char>(result.front())))
	{
		result = fallback + (result.empty() ? std::string() : std::string("_") + result);
	}

	return result;
}

std::string Texture::GetAtlasUVTransformUniformName() const
{
	return GetShaderUniformName() + "_UVTransform";
}

GEuint Texture::GetEffectiveRendererTextureId() const
{
	return atlasTexture_ ? atlasTexture_->GetOwnedRendererTextureId() : rendererTextureId_;
}

void Texture::UpdateBufferOnGPU()
{
	if (rendererTextureId_ == 0 || atlasTexture_)
	{
		return;
	}

	UpdateSizeOnGPU();
}

void Texture::ClearBuffer()
{
	delete[] buffer_;
	buffer_ = nullptr;
}
