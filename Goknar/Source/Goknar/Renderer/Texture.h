#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "Goknar/Core.h"
#include "Goknar/Managers/ObjectIDManager.h"
#include "Types.h"

#include <string>

class Shader;
class Image;

enum class TextureBindTarget
{
	TEXTURE_1D = GL_TEXTURE_1D,
	TEXTURE_2D = GL_TEXTURE_2D,
	TEXTURE_3D = GL_TEXTURE_3D,
	TEXTURE_RECTANGLE = GL_TEXTURE_RECTANGLE,
	TEXTURE_BUFFER = GL_TEXTURE_BUFFER,
	TEXTURE_CUBE_MAP = GL_TEXTURE_CUBE_MAP,
	TEXTURE_1D_ARRAY = GL_TEXTURE_1D_ARRAY,
	TEXTURE_2D_ARRAY = GL_TEXTURE_2D_ARRAY,
	TEXTURE_CUBE_MAP_ARRAY = GL_TEXTURE_CUBE_MAP_ARRAY,
	TEXTURE_2D_MULTISAMPLE = GL_TEXTURE_2D_MULTISAMPLE,
	TEXTURE_2D_MULTISAMPLE_ARRAY = GL_TEXTURE_2D_MULTISAMPLE_ARRAY
};

enum class TextureImageTarget
{
	TEXTURE_2D = GL_TEXTURE_2D,
	PROXY_TEXTURE_2D = GL_PROXY_TEXTURE_2D,

	TEXTURE_1D_ARRAY = GL_TEXTURE_1D_ARRAY,
	PROXY_TEXTURE_1D_ARRAY = GL_PROXY_TEXTURE_1D_ARRAY,

	TEXTURE_RECTANGLE = GL_TEXTURE_RECTANGLE,
	PROXY_TEXTURE_CUBE_MAP = GL_PROXY_TEXTURE_CUBE_MAP,

	TEXTURE_CUBE_MAP_POSITIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	TEXTURE_CUBE_MAP_NEGATIVE_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	TEXTURE_CUBE_MAP_POSITIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	TEXTURE_CUBE_MAP_NEGATIVE_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	TEXTURE_CUBE_MAP_POSITIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	TEXTURE_CUBE_MAP_NEGATIVE_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
	PROXY_TEXTURE_RECTANGLE = GL_PROXY_TEXTURE_RECTANGLE,
};

enum class TextureWrapping
{
	REPEAT = GL_REPEAT,
	MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
	CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
	CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER
};

enum class TextureMinFilter
{
	NONE = GL_NONE,
	NEAREST = GL_NEAREST,
	LINEAR = GL_LINEAR,
	NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
	LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR
};

enum class TextureMagFilter
{
	NONE = -1,
	NEAREST = GL_NEAREST,
	LINEAR = GL_LINEAR
};

enum class TextureFormat
{
	DEPTH = GL_DEPTH_COMPONENT,
	DEPTH_STENCIL = GL_DEPTH_STENCIL,
	RED = GL_RED,
	RG = GL_RG,
	RGB = GL_RGB,
	RGBA = GL_RGBA
};

enum class TextureInternalFormat
{
	DEPTH = GL_DEPTH_COMPONENT,
	DEPTH_16 = GL_DEPTH_COMPONENT16,
	DEPTH_24 = GL_DEPTH_COMPONENT24,
	DEPTH_32 = GL_DEPTH_COMPONENT32,
	DEPTH_32F = GL_DEPTH_COMPONENT32F,
	DEPTH_STENCIL = GL_DEPTH_STENCIL,
	DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,
	RED = GL_RED,
	RG = GL_RG,
	RGB = GL_RGB,
	RGB16F = GL_RGB16F,
	RGB32F = GL_RGB32F,
	RGBA = GL_RGBA,
	RGBA16F = GL_RGBA16F,
	RGBA32F = GL_RGBA32F,
};

enum class TextureType
{
	UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
	BYTE = GL_BYTE,
	UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
	SHORT = GL_SHORT,
	UNSIGNED_INT = GL_UNSIGNED_INT,
	INT = GL_INT,
	HALF_FLOAT = GL_HALF_FLOAT,
	FLOAT = GL_FLOAT,
	UNSIGNED_BYTE_3_3_2 = GL_UNSIGNED_BYTE_3_3_2,
	UNSIGNED_BYTE_2_3_3_REV = GL_UNSIGNED_BYTE_2_3_3_REV,
	UNSIGNED_SHORT_5_6_5 = GL_UNSIGNED_SHORT_5_6_5,
	UNSIGNED_SHORT_5_6_5_REV = GL_UNSIGNED_SHORT_5_6_5_REV,
	UNSIGNED_SHORT_4_4_4_4 = GL_UNSIGNED_SHORT_4_4_4_4,
	UNSIGNED_SHORT_4_4_4_4_REV = GL_UNSIGNED_SHORT_4_4_4_4_REV,
	UNSIGNED_SHORT_5_5_5_1 = GL_UNSIGNED_SHORT_5_5_5_1,
	UNSIGNED_SHORT_1_5_5_5_REV = GL_UNSIGNED_SHORT_1_5_5_5_REV,
	UNSIGNED_INT_8_8_8_8 = GL_UNSIGNED_INT_8_8_8_8,
	UNSIGNED_INT_8_8_8_8_REV = GL_UNSIGNED_INT_8_8_8_8_REV,
	UNSIGNED_INT_10_10_10_2 = GL_UNSIGNED_INT_10_10_10_2,
	UNSIGNED_INT_2_10_10_10_REV = GL_UNSIGNED_INT_2_10_10_10_REV
};

enum class TextureCompareMode
{
	NONE = GL_NONE,
	COMPARE_REF_TO_TEXTURE = GL_COMPARE_REF_TO_TEXTURE
};

enum class TextureCompareFunc
{
	LEQUAL = GL_LEQUAL,
	GEQUAL = GL_GEQUAL,
	LESS = GL_LESS,
	GREATER = GL_GREATER,
	EQUAL = GL_EQUAL,
	NOTEQUAL = GL_NOTEQUAL,
	ALWAYS = GL_ALWAYS,
	NEVER = GL_NEVER
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
	void Unbind();

	bool LoadTextureImage();

	void ReadFromFrameBuffer(GEuint framebuffer);
	void Save(std::string path);

	GEuint GetRendererTextureId()
	{
		return rendererTextureId_;
	}

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

	void SetName(const std::string& name)
	{
		name_ = name;
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

	std::string name_{ "" };
	std::string imagePath_{ "" };
	const unsigned char* buffer_{ nullptr };
	GEuint rendererTextureId_{ 0 };

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
};

#endif