#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "Goknar/Core.h"
#include "Goknar/Managers/ObjectIDManager.h"
#include "Goknar/Renderer/TextureAtlasTypes.h"
#include "Types.h"

#include <string>

class Shader;
class Image;

enum class TextureImageAccess
{
	READ_ONLY,
	WRITE_ONLY,
	READ_WRITE
};

enum class TextureBindTarget
{
	TEXTURE_1D,
	TEXTURE_2D,
	TEXTURE_3D,
	TEXTURE_RECTANGLE,
	TEXTURE_BUFFER,
	TEXTURE_CUBE_MAP,
	TEXTURE_1D_ARRAY,
	TEXTURE_2D_ARRAY,
	TEXTURE_CUBE_MAP_ARRAY,
	TEXTURE_2D_MULTISAMPLE,
	TEXTURE_2D_MULTISAMPLE_ARRAY
};

enum class TextureImageTarget
{
	TEXTURE_2D,
	PROXY_TEXTURE_2D,

	TEXTURE_1D_ARRAY,
	PROXY_TEXTURE_1D_ARRAY,

	TEXTURE_RECTANGLE,
	PROXY_TEXTURE_CUBE_MAP,

	TEXTURE_CUBE_MAP_POSITIVE_X,
	TEXTURE_CUBE_MAP_NEGATIVE_X,
	TEXTURE_CUBE_MAP_POSITIVE_Y,
	TEXTURE_CUBE_MAP_NEGATIVE_Y,
	TEXTURE_CUBE_MAP_POSITIVE_Z,
	TEXTURE_CUBE_MAP_NEGATIVE_Z,
	PROXY_TEXTURE_RECTANGLE,
};

enum class TextureWrapping
{
	REPEAT,
	MIRRORED_REPEAT,
	CLAMP_TO_EDGE,
	CLAMP_TO_BORDER
};

enum class TextureMinFilter
{
	NONE,
	NEAREST,
	LINEAR,
	NEAREST_MIPMAP_NEAREST,
	LINEAR_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_LINEAR
};

enum class TextureMagFilter
{
	NONE = -1,
	NEAREST,
	LINEAR
};

enum class TextureFormat
{
	DEPTH,
	DEPTH_STENCIL,
	RED,
	RG,
	RGB,
	RGBA
};

enum class TextureInternalFormat
{
	DEPTH,
	DEPTH_16,
	DEPTH_24,
	DEPTH_32,
	DEPTH_32F,
	DEPTH_STENCIL,
	DEPTH24_STENCIL8,
	RED,
	RG,
	RGB,
	RGB16F,
	RGB32F,
	RGBA,
	RGBA16F,
	RGBA32F,
};

enum class TextureType
{
	UNSIGNED_BYTE,
	BYTE,
	UNSIGNED_SHORT,
	SHORT,
	UNSIGNED_INT,
	INT,
	HALF_FLOAT,
	FLOAT,
	UNSIGNED_BYTE_3_3_2,
	UNSIGNED_BYTE_2_3_3_REV,
	UNSIGNED_SHORT_5_6_5,
	UNSIGNED_SHORT_5_6_5_REV,
	UNSIGNED_SHORT_4_4_4_4,
	UNSIGNED_SHORT_4_4_4_4_REV,
	UNSIGNED_SHORT_5_5_5_1,
	UNSIGNED_SHORT_1_5_5_5_REV,
	UNSIGNED_INT_8_8_8_8,
	UNSIGNED_INT_8_8_8_8_REV,
	UNSIGNED_INT_10_10_10_2,
	UNSIGNED_INT_2_10_10_10_REV
};

enum class TextureCompareMode
{
	NONE,
	COMPARE_REF_TO_TEXTURE
};

enum class TextureCompareFunc
{
	LEQUAL,
	GEQUAL,
	LESS,
	GREATER,
	EQUAL,
	NOTEQUAL,
	ALWAYS,
	NEVER
};

enum class TextureDataType : unsigned char
{
	STATIC = 0,
	DYNAMIC
};

enum class TextureUsage : unsigned char
{
	None = 0,
	Diffuse,
	Normal,
	AmbientOcclusion,
	Metallic,
	Specular,
	Emmisive,
	Roughness,
	Height
};


class GOKNAR_API Texture
{
public:
	Texture();
	Texture(std::string imagePath);
	Texture(Image* image);

	virtual ~Texture();

	void PreInit();
	void Init();
	void PostInit();

	void Bind(const Shader* shader = nullptr) const;
	void BindToTextureUnit(unsigned int textureUnit) const;
	void BindAsImage(unsigned int imageUnit, TextureImageAccess access) const;
	void Unbind();

	bool LoadTextureImage();

	void ReadFromFrameBuffer(GEuint framebuffer);
	void Save(std::string path);
	void GenerateMipmap() const;

	GEuint GetRendererTextureId() const
	{
		return GetEffectiveRendererTextureId();
	}

	GEuint GetOwnedRendererTextureId() const
	{
		return rendererTextureId_;
	}

	bool GetIsInitialized() const
	{
		return isInitialized_;
	}

	void UpdateBufferOnGPU();
	void ClearBuffer();

	int GetWidth() const
	{
		return width_;
	}

	void SetWidth(int width)
	{
		width_ = width;
		if(isInitialized_)
		{
			UpdateSizeOnGPU();
		}
	}

	int GetHeight() const
	{
		return height_;
	}

	void SetHeight(int height)
	{
		height_ = height;
		if(isInitialized_)
		{
			UpdateSizeOnGPU();
		}
	}

	void SetSize(int width, int height)
	{
		width_ = width;
		height_ = height;

		if(isInitialized_)
		{
			UpdateSizeOnGPU();
		}
	}

	void SetTextureImagePath(const std::string& imagePath)
	{
		imagePath_ = ContentDir + imagePath;
	}

	void SetTextureImagePathAbsolute(const std::string& imagePath)
	{
		imagePath_ = imagePath;
	}

	const std::string& GetTextureImagePath() const
	{
		return imagePath_;
	}

	GEuint GetGUID() const
	{
		return GUID_;
	}

	const std::string& GetName() const
	{
		return name_;
	}

	std::string GetShaderUniformName() const;

	void SetName(const std::string& name)
	{
		name_ = name;
	}

	void SetAtlasTexture(
		Texture* atlasTexture,
		float uMin,
		float vMin,
		float uMax,
		float vMax,
		TextureAtlasCategory atlasCategory = TextureAtlasCategory::Opaque,
		int atlasIndex = -1);

	void SetWaitsForTextureAtlas(bool waitsForTextureAtlas)
	{
		waitsForTextureAtlas_ = waitsForTextureAtlas;
	}

	bool GetWaitsForTextureAtlas() const
	{
		return waitsForTextureAtlas_;
	}

	bool GetUsesAtlasTexture() const
	{
		return atlasTexture_ != nullptr;
	}

	const Texture* GetAtlasTexture() const
	{
		return atlasTexture_;
	}

	void SetTextureAtlasCategory(TextureAtlasCategory atlasCategory)
	{
		atlasCategory_ = atlasCategory;
	}

	TextureAtlasCategory GetTextureAtlasCategory() const
	{
		return atlasCategory_;
	}

	int GetTextureAtlasIndex() const
	{
		return atlasIndex_;
	}

	void SetTextureAtlasProxySourceImage(Image* image);

	Image* GetTextureAtlasProxySourceImage() const
	{
		return atlasProxySourceImage_;
	}

	std::string GetAtlasUVTransformUniformName() const;

	float GetAtlasUScale() const
	{
		return atlasUScale_;
	}

	float GetAtlasVScale() const
	{
		return atlasVScale_;
	}

	float GetAtlasUOffset() const
	{
		return atlasUOffset_;
	}

	float GetAtlasVOffset() const
	{
		return atlasVOffset_;
	}

	TextureBindTarget GetTextureBindTarget() const
	{
		return textureBindTarget_;
	}

	void SetTextureBindTarget(TextureBindTarget textureTarget)
	{
		textureBindTarget_ = textureTarget;
	}

	TextureImageTarget GetTextureImageTarget() const
	{
		return textureImageTarget_;
	}

	void SetTextureImageTarget(TextureImageTarget textureImageTarget)
	{
		textureImageTarget_ = textureImageTarget;
	}

	TextureWrapping GetTextureWrappingS() const
	{
		return textureWrappingS_;
	}

	void SetTextureWrappingS(TextureWrapping textureWrappingS)
	{
		textureWrappingS_ = textureWrappingS;
	}

	TextureWrapping GetTextureWrappingT() const
	{
		return textureWrappingT_;
	}

	void SetTextureWrappingT(TextureWrapping textureWrappingT)
	{
		textureWrappingT_ = textureWrappingT;
	}

	TextureWrapping GetTextureWrappingR() const
	{
		return textureWrappingR_;
	}

	void SetTextureWrappingR(TextureWrapping textureWrappingR)
	{
		textureWrappingR_ = textureWrappingR;
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

	TextureFormat GetTextureFormat() const
	{
		return textureFormat_;
	}

	void SetTextureFormat(TextureFormat textureFormat)
	{
		textureFormat_ = textureFormat;
	}

	TextureInternalFormat GetTextureInternalFormat() const
	{
		return textureInternalFormat_;
	}

	void SetTextureInternalFormat(TextureInternalFormat textureInternalFormat)
	{
		textureInternalFormat_ = textureInternalFormat;
	}

	TextureDataType GetTextureDataType() const
	{
		return textureDataType_;
	}

	void SetTextureDataType(TextureDataType textureDataType)
	{
		textureDataType_ = textureDataType;
	}

	TextureType GetTextureType() const
	{
		return textureType_;
	}

	void SetTextureType(TextureType textureType)
	{
		textureType_ = textureType;
	}

	TextureCompareMode GetTextureCompareMode() const
	{
		return textureCompareMode_;
	}

	void SetTextureCompareMode(TextureCompareMode textureCompareMode)
	{
		textureCompareMode_ = textureCompareMode;
	}

	TextureCompareFunc GetTextureCompareFunc() const
	{
		return textureCompareFunc_;
	}

	void SetTextureCompareFunc(TextureCompareFunc textureCompareFunc)
	{
		textureCompareFunc_ = textureCompareFunc;
	}

	TextureUsage GetTextureUsage() const
	{
		return textureUsage_;
	}

	void SetTextureUsage(TextureUsage textureUsage)
	{
		textureUsage_ = textureUsage;
	}

	void SetChannels(int channels)
	{
		channels_ = channels;
	}

	int GetChannels() const
	{
		return channels_;
	}

	void SetBuffer(unsigned char* buffer)
	{
		buffer_ = buffer;
	}

	const unsigned char* GetBuffer()
	{
		return buffer_;
	}

	void SetGenerateMipmap(bool generateMipmap)
	{
		generateMipmap_ = generateMipmap;
	}

	bool GetGenerateMipmap() const
	{
		return generateMipmap_;
	}

protected:

private:
	void UpdateSizeOnGPU();
	GEuint GetEffectiveRendererTextureId() const;

	std::string name_{ "" };
	std::string imagePath_{ "" };
	const unsigned char* buffer_{ nullptr };
	GEuint rendererTextureId_{ 0 };
	Texture* atlasTexture_{ nullptr };
	Image* atlasProxySourceImage_{ nullptr };
	TextureAtlasCategory atlasCategory_{ TextureAtlasCategory::Opaque };
	int atlasIndex_{ -1 };
	float atlasUScale_{ 1.0f };
	float atlasVScale_{ 1.0f };
	float atlasUOffset_{ 0.0f };
	float atlasVOffset_{ 0.0f };

	TextureBindTarget textureBindTarget_{ TextureBindTarget::TEXTURE_2D};
	TextureImageTarget textureImageTarget_{ TextureImageTarget::TEXTURE_2D };
	TextureWrapping textureWrappingS_{ TextureWrapping::REPEAT };
	TextureWrapping textureWrappingT_{ TextureWrapping::REPEAT };
	TextureWrapping textureWrappingR_{ TextureWrapping::REPEAT };
	TextureMinFilter minFilter_{ TextureMinFilter::LINEAR };
	TextureMagFilter magFilter_{ TextureMagFilter::LINEAR };
	TextureFormat textureFormat_{ TextureFormat::RGB };
	TextureInternalFormat textureInternalFormat_{ TextureInternalFormat::RGB };
	TextureType textureType_{ TextureType::UNSIGNED_BYTE };
	TextureCompareMode textureCompareMode_{ TextureCompareMode::NONE };
	TextureCompareFunc textureCompareFunc_{ TextureCompareFunc::LEQUAL };

	TextureDataType textureDataType_{ TextureDataType::STATIC };

	TextureUsage textureUsage_{ TextureUsage::Diffuse };

	int GUID_{ 0 };
	int width_{ 0 };
	int height_{ 0 };
	int channels_{ 0 };

	bool isInitialized_{ false };
	bool generateMipmap_{ true };
	bool waitsForTextureAtlas_{ false };
};

#endif
