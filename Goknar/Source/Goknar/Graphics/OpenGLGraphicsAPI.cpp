#include "pch.h"

#include "OpenGLGraphicsAPI.h"

#include "Goknar/Log.h"
#include "Goknar/Renderer/Framebuffer.h"
#include "Goknar/Renderer/RenderBuffer.h"
#include "Goknar/Renderer/Texture.h"

#include "glad/glad.h"

#include <cstdlib>

namespace
{
	GLenum ToOpenGLCapability(GraphicsCapability capability)
	{
		switch (capability)
		{
		case GraphicsCapability::DepthTest: return GL_DEPTH_TEST;
		case GraphicsCapability::Blend: return GL_BLEND;
		case GraphicsCapability::CullFace: return GL_CULL_FACE;
		case GraphicsCapability::Multisample: return GL_MULTISAMPLE;
		default: return GL_NONE;
		}
	}

	GLenum ToOpenGLDepthFunction(GraphicsDepthFunction depthFunction)
	{
		switch (depthFunction)
		{
		case GraphicsDepthFunction::Lequal: return GL_LEQUAL;
		default: return GL_LEQUAL;
		}
	}

	GLenum ToOpenGLFrontFace(GraphicsFrontFace frontFace)
	{
		switch (frontFace)
		{
		case GraphicsFrontFace::CounterClockwise: return GL_CCW;
		default: return GL_CCW;
		}
	}

	GLenum ToOpenGLBlendFactor(GraphicsBlendFactor blendFactor)
	{
		switch (blendFactor)
		{
		case GraphicsBlendFactor::SourceAlpha: return GL_SRC_ALPHA;
		case GraphicsBlendFactor::OneMinusSourceAlpha: return GL_ONE_MINUS_SRC_ALPHA;
		default: return GL_ONE_MINUS_SRC_ALPHA;
		}
	}

	GLbitfield ToOpenGLClearFlags(GraphicsClearBufferFlags buffers)
	{
		GLbitfield result = 0;
		if (buffers & static_cast<GraphicsClearBufferFlags>(GraphicsClearBuffer::Color))
		{
			result |= GL_COLOR_BUFFER_BIT;
		}
		if (buffers & static_cast<GraphicsClearBufferFlags>(GraphicsClearBuffer::Depth))
		{
			result |= GL_DEPTH_BUFFER_BIT;
		}
		return result;
	}

	GLenum ToOpenGLBufferTarget(GraphicsBufferTarget target)
	{
		switch (target)
		{
		case GraphicsBufferTarget::ArrayBuffer: return GL_ARRAY_BUFFER;
		case GraphicsBufferTarget::ElementArrayBuffer: return GL_ELEMENT_ARRAY_BUFFER;
		case GraphicsBufferTarget::UniformBuffer: return GL_UNIFORM_BUFFER;
		case GraphicsBufferTarget::ShaderStorageBuffer: return GL_SHADER_STORAGE_BUFFER;
		case GraphicsBufferTarget::DrawIndirectBuffer: return GL_DRAW_INDIRECT_BUFFER;
		default: return GL_ARRAY_BUFFER;
		}
	}

	GLenum ToOpenGLBufferUsage(GraphicsBufferUsage usage)
	{
		switch (usage)
		{
		case GraphicsBufferUsage::StaticDraw: return GL_STATIC_DRAW;
		case GraphicsBufferUsage::DynamicDraw: return GL_DYNAMIC_DRAW;
		default: return GL_STATIC_DRAW;
		}
	}

	GLenum ToOpenGLPrimitive(GraphicsPrimitive primitive)
	{
		switch (primitive)
		{
		case GraphicsPrimitive::Triangles: return GL_TRIANGLES;
		default: return GL_TRIANGLES;
		}
	}

	GLenum ToOpenGLDataType(GraphicsDataType type)
	{
		switch (type)
		{
		case GraphicsDataType::Float: return GL_FLOAT;
		case GraphicsDataType::UnsignedInt: return GL_UNSIGNED_INT;
		default: return GL_FLOAT;
		}
	}

	GLenum ToOpenGLShaderStage(GraphicsShaderStage shaderStage)
	{
		switch (shaderStage)
		{
		case GraphicsShaderStage::Vertex: return GL_VERTEX_SHADER;
		case GraphicsShaderStage::Fragment: return GL_FRAGMENT_SHADER;
		case GraphicsShaderStage::Geometry: return GL_GEOMETRY_SHADER;
		case GraphicsShaderStage::Compute: return GL_COMPUTE_SHADER;
		default: return GL_VERTEX_SHADER;
		}
	}

	GLbitfield ToOpenGLMemoryBarrierFlags(GraphicsMemoryBarrierFlags flags)
	{
		GLbitfield result = 0;
		if (flags & static_cast<GraphicsMemoryBarrierFlags>(GraphicsMemoryBarrier::ShaderStorage))
		{
			result |= GL_SHADER_STORAGE_BARRIER_BIT;
		}
		if (flags & static_cast<GraphicsMemoryBarrierFlags>(GraphicsMemoryBarrier::Command))
		{
			result |= GL_COMMAND_BARRIER_BIT;
		}
		if (flags & static_cast<GraphicsMemoryBarrierFlags>(GraphicsMemoryBarrier::ShaderImageAccess))
		{
			result |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
		}
		if (flags & static_cast<GraphicsMemoryBarrierFlags>(GraphicsMemoryBarrier::TextureFetch))
		{
			result |= GL_TEXTURE_FETCH_BARRIER_BIT;
		}
		return result;
	}

	GLenum ToOpenGLPixelStoreParameter(GraphicsPixelStoreParameter parameter)
	{
		switch (parameter)
		{
		case GraphicsPixelStoreParameter::UnpackAlignment: return GL_UNPACK_ALIGNMENT;
		default: return GL_UNPACK_ALIGNMENT;
		}
	}

	GLenum ToOpenGLTextureBindTarget(TextureBindTarget target)
	{
		switch (target)
		{
		case TextureBindTarget::TEXTURE_1D: return GL_TEXTURE_1D;
		case TextureBindTarget::TEXTURE_2D: return GL_TEXTURE_2D;
		case TextureBindTarget::TEXTURE_3D: return GL_TEXTURE_3D;
		case TextureBindTarget::TEXTURE_RECTANGLE: return GL_TEXTURE_RECTANGLE;
		case TextureBindTarget::TEXTURE_BUFFER: return GL_TEXTURE_BUFFER;
		case TextureBindTarget::TEXTURE_CUBE_MAP: return GL_TEXTURE_CUBE_MAP;
		case TextureBindTarget::TEXTURE_1D_ARRAY: return GL_TEXTURE_1D_ARRAY;
		case TextureBindTarget::TEXTURE_2D_ARRAY: return GL_TEXTURE_2D_ARRAY;
		case TextureBindTarget::TEXTURE_CUBE_MAP_ARRAY: return GL_TEXTURE_CUBE_MAP_ARRAY;
		case TextureBindTarget::TEXTURE_2D_MULTISAMPLE: return GL_TEXTURE_2D_MULTISAMPLE;
		case TextureBindTarget::TEXTURE_2D_MULTISAMPLE_ARRAY: return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
		default: return GL_TEXTURE_2D;
		}
	}

	GLenum ToOpenGLTextureImageTarget(TextureImageTarget target)
	{
		switch (target)
		{
		case TextureImageTarget::TEXTURE_3D: return GL_TEXTURE_3D;
		case TextureImageTarget::TEXTURE_2D: return GL_TEXTURE_2D;
		case TextureImageTarget::PROXY_TEXTURE_2D: return GL_PROXY_TEXTURE_2D;
		case TextureImageTarget::TEXTURE_1D_ARRAY: return GL_TEXTURE_1D_ARRAY;
		case TextureImageTarget::PROXY_TEXTURE_1D_ARRAY: return GL_PROXY_TEXTURE_1D_ARRAY;
		case TextureImageTarget::TEXTURE_RECTANGLE: return GL_TEXTURE_RECTANGLE;
		case TextureImageTarget::PROXY_TEXTURE_CUBE_MAP: return GL_PROXY_TEXTURE_CUBE_MAP;
		case TextureImageTarget::TEXTURE_CUBE_MAP_POSITIVE_X: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		case TextureImageTarget::TEXTURE_CUBE_MAP_NEGATIVE_X: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
		case TextureImageTarget::TEXTURE_CUBE_MAP_POSITIVE_Y: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
		case TextureImageTarget::TEXTURE_CUBE_MAP_NEGATIVE_Y: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
		case TextureImageTarget::TEXTURE_CUBE_MAP_POSITIVE_Z: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
		case TextureImageTarget::TEXTURE_CUBE_MAP_NEGATIVE_Z: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
		case TextureImageTarget::PROXY_TEXTURE_RECTANGLE: return GL_PROXY_TEXTURE_RECTANGLE;
		default: return GL_TEXTURE_2D;
		}
	}

	GLenum ToOpenGLTextureWrapping(TextureWrapping wrapping)
	{
		switch (wrapping)
		{
		case TextureWrapping::REPEAT: return GL_REPEAT;
		case TextureWrapping::MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
		case TextureWrapping::CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
		case TextureWrapping::CLAMP_TO_BORDER: return GL_CLAMP_TO_BORDER;
		default: return GL_REPEAT;
		}
	}

	GLenum ToOpenGLTextureMinFilter(TextureMinFilter filter)
	{
		switch (filter)
		{
		case TextureMinFilter::NONE: return GL_NONE;
		case TextureMinFilter::NEAREST: return GL_NEAREST;
		case TextureMinFilter::LINEAR: return GL_LINEAR;
		case TextureMinFilter::NEAREST_MIPMAP_NEAREST: return GL_NEAREST_MIPMAP_NEAREST;
		case TextureMinFilter::LINEAR_MIPMAP_NEAREST: return GL_LINEAR_MIPMAP_NEAREST;
		case TextureMinFilter::NEAREST_MIPMAP_LINEAR: return GL_NEAREST_MIPMAP_LINEAR;
		case TextureMinFilter::LINEAR_MIPMAP_LINEAR: return GL_LINEAR_MIPMAP_LINEAR;
		default: return GL_LINEAR;
		}
	}

	GLenum ToOpenGLTextureMagFilter(TextureMagFilter filter)
	{
		switch (filter)
		{
		case TextureMagFilter::NONE: return GL_NONE;
		case TextureMagFilter::NEAREST: return GL_NEAREST;
		case TextureMagFilter::LINEAR: return GL_LINEAR;
		default: return GL_LINEAR;
		}
	}

	GLenum ToOpenGLTextureFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::DEPTH: return GL_DEPTH_COMPONENT;
		case TextureFormat::DEPTH_STENCIL: return GL_DEPTH_STENCIL;
		case TextureFormat::RED: return GL_RED;
		case TextureFormat::RG: return GL_RG;
		case TextureFormat::RGB: return GL_RGB;
		case TextureFormat::RGBA: return GL_RGBA;
		default: return GL_RGB;
		}
	}

	GLenum ToOpenGLTextureInternalFormat(TextureInternalFormat format)
	{
		switch (format)
		{
		case TextureInternalFormat::DEPTH: return GL_DEPTH_COMPONENT;
		case TextureInternalFormat::DEPTH_16: return GL_DEPTH_COMPONENT16;
		case TextureInternalFormat::DEPTH_24: return GL_DEPTH_COMPONENT24;
		case TextureInternalFormat::DEPTH_32: return GL_DEPTH_COMPONENT32;
		case TextureInternalFormat::DEPTH_32F: return GL_DEPTH_COMPONENT32F;
		case TextureInternalFormat::DEPTH_STENCIL: return GL_DEPTH_STENCIL;
		case TextureInternalFormat::DEPTH24_STENCIL8: return GL_DEPTH24_STENCIL8;
		case TextureInternalFormat::RED: return GL_RED;
		case TextureInternalFormat::RG: return GL_RG;
		case TextureInternalFormat::RGB: return GL_RGB;
		case TextureInternalFormat::RGB16F: return GL_RGB16F;
		case TextureInternalFormat::RGB32F: return GL_RGB32F;
		case TextureInternalFormat::RGBA: return GL_RGBA;
		case TextureInternalFormat::RGBA16F: return GL_RGBA16F;
		case TextureInternalFormat::RGBA32F: return GL_RGBA32F;
		default: return GL_RGB;
		}
	}

	GLenum ToOpenGLTextureType(TextureType type)
	{
		switch (type)
		{
		case TextureType::UNSIGNED_BYTE: return GL_UNSIGNED_BYTE;
		case TextureType::BYTE: return GL_BYTE;
		case TextureType::UNSIGNED_SHORT: return GL_UNSIGNED_SHORT;
		case TextureType::SHORT: return GL_SHORT;
		case TextureType::UNSIGNED_INT: return GL_UNSIGNED_INT;
		case TextureType::INT: return GL_INT;
		case TextureType::HALF_FLOAT: return GL_HALF_FLOAT;
		case TextureType::FLOAT: return GL_FLOAT;
		case TextureType::UNSIGNED_BYTE_3_3_2: return GL_UNSIGNED_BYTE_3_3_2;
		case TextureType::UNSIGNED_BYTE_2_3_3_REV: return GL_UNSIGNED_BYTE_2_3_3_REV;
		case TextureType::UNSIGNED_SHORT_5_6_5: return GL_UNSIGNED_SHORT_5_6_5;
		case TextureType::UNSIGNED_SHORT_5_6_5_REV: return GL_UNSIGNED_SHORT_5_6_5_REV;
		case TextureType::UNSIGNED_SHORT_4_4_4_4: return GL_UNSIGNED_SHORT_4_4_4_4;
		case TextureType::UNSIGNED_SHORT_4_4_4_4_REV: return GL_UNSIGNED_SHORT_4_4_4_4_REV;
		case TextureType::UNSIGNED_SHORT_5_5_5_1: return GL_UNSIGNED_SHORT_5_5_5_1;
		case TextureType::UNSIGNED_SHORT_1_5_5_5_REV: return GL_UNSIGNED_SHORT_1_5_5_5_REV;
		case TextureType::UNSIGNED_INT_8_8_8_8: return GL_UNSIGNED_INT_8_8_8_8;
		case TextureType::UNSIGNED_INT_8_8_8_8_REV: return GL_UNSIGNED_INT_8_8_8_8_REV;
		case TextureType::UNSIGNED_INT_10_10_10_2: return GL_UNSIGNED_INT_10_10_10_2;
		case TextureType::UNSIGNED_INT_2_10_10_10_REV: return GL_UNSIGNED_INT_2_10_10_10_REV;
		default: return GL_UNSIGNED_BYTE;
		}
	}

	GLenum ToOpenGLTextureCompareMode(TextureCompareMode compareMode)
	{
		switch (compareMode)
		{
		case TextureCompareMode::NONE: return GL_NONE;
		case TextureCompareMode::COMPARE_REF_TO_TEXTURE: return GL_COMPARE_REF_TO_TEXTURE;
		default: return GL_NONE;
		}
	}

	GLenum ToOpenGLTextureCompareFunc(TextureCompareFunc compareFunc)
	{
		switch (compareFunc)
		{
		case TextureCompareFunc::LEQUAL: return GL_LEQUAL;
		case TextureCompareFunc::GEQUAL: return GL_GEQUAL;
		case TextureCompareFunc::LESS: return GL_LESS;
		case TextureCompareFunc::GREATER: return GL_GREATER;
		case TextureCompareFunc::EQUAL: return GL_EQUAL;
		case TextureCompareFunc::NOTEQUAL: return GL_NOTEQUAL;
		case TextureCompareFunc::ALWAYS: return GL_ALWAYS;
		case TextureCompareFunc::NEVER: return GL_NEVER;
		default: return GL_LEQUAL;
		}
	}

	GLenum ToOpenGLTextureImageAccess(TextureImageAccess access)
	{
		switch (access)
		{
		case TextureImageAccess::READ_ONLY: return GL_READ_ONLY;
		case TextureImageAccess::WRITE_ONLY: return GL_WRITE_ONLY;
		case TextureImageAccess::READ_WRITE: return GL_READ_WRITE;
		default: return GL_READ_ONLY;
		}
	}

	GLenum ToOpenGLFrameBufferBindTarget(FrameBufferBindTarget target)
	{
		switch (target)
		{
		case FrameBufferBindTarget::DRAW_FRAMEBUFFER: return GL_DRAW_FRAMEBUFFER;
		case FrameBufferBindTarget::READ_FRAMEBUFFER: return GL_READ_FRAMEBUFFER;
		case FrameBufferBindTarget::FRAMEBUFFER: return GL_FRAMEBUFFER;
		default: return GL_FRAMEBUFFER;
		}
	}

	GLenum ToOpenGLFrameBufferAttachment(FrameBufferAttachment attachment)
	{
		switch (attachment)
		{
		case FrameBufferAttachment::COLOR_ATTACHMENT0: return GL_COLOR_ATTACHMENT0;
		case FrameBufferAttachment::COLOR_ATTACHMENT1: return GL_COLOR_ATTACHMENT1;
		case FrameBufferAttachment::COLOR_ATTACHMENT2: return GL_COLOR_ATTACHMENT2;
		case FrameBufferAttachment::COLOR_ATTACHMENT3: return GL_COLOR_ATTACHMENT3;
		case FrameBufferAttachment::COLOR_ATTACHMENT4: return GL_COLOR_ATTACHMENT4;
		case FrameBufferAttachment::COLOR_ATTACHMENT5: return GL_COLOR_ATTACHMENT5;
		case FrameBufferAttachment::COLOR_ATTACHMENT6: return GL_COLOR_ATTACHMENT6;
		case FrameBufferAttachment::COLOR_ATTACHMENT7: return GL_COLOR_ATTACHMENT7;
		case FrameBufferAttachment::COLOR_ATTACHMENT8: return GL_COLOR_ATTACHMENT8;
		case FrameBufferAttachment::COLOR_ATTACHMENT9: return GL_COLOR_ATTACHMENT9;
		case FrameBufferAttachment::DEPTH_ATTACHMENT: return GL_DEPTH_ATTACHMENT;
		case FrameBufferAttachment::STENCIL_ATTACHMENT: return GL_STENCIL_ATTACHMENT;
		default: return GL_COLOR_ATTACHMENT0;
		}
	}

	GLenum ToOpenGLRenderBufferBindTarget(RenderBufferBindTarget target)
	{
		switch (target)
		{
		case RenderBufferBindTarget::RENDERBUFFER: return GL_RENDERBUFFER;
		default: return GL_RENDERBUFFER;
		}
	}

	GLenum ToOpenGLRenderBufferAttachment(RenderBufferAttachment attachment)
	{
		switch (attachment)
		{
		case RenderBufferAttachment::COLOR_ATTACHMENT0: return GL_COLOR_ATTACHMENT0;
		case RenderBufferAttachment::COLOR_ATTACHMENT1: return GL_COLOR_ATTACHMENT1;
		case RenderBufferAttachment::COLOR_ATTACHMENT2: return GL_COLOR_ATTACHMENT2;
		case RenderBufferAttachment::COLOR_ATTACHMENT3: return GL_COLOR_ATTACHMENT3;
		case RenderBufferAttachment::COLOR_ATTACHMENT4: return GL_COLOR_ATTACHMENT4;
		case RenderBufferAttachment::COLOR_ATTACHMENT5: return GL_COLOR_ATTACHMENT5;
		case RenderBufferAttachment::COLOR_ATTACHMENT6: return GL_COLOR_ATTACHMENT6;
		case RenderBufferAttachment::COLOR_ATTACHMENT7: return GL_COLOR_ATTACHMENT7;
		case RenderBufferAttachment::COLOR_ATTACHMENT8: return GL_COLOR_ATTACHMENT8;
		case RenderBufferAttachment::COLOR_ATTACHMENT9: return GL_COLOR_ATTACHMENT9;
		case RenderBufferAttachment::DEPTH_ATTACHMENT: return GL_DEPTH_ATTACHMENT;
		case RenderBufferAttachment::STENCIL_ATTACHMENT: return GL_STENCIL_ATTACHMENT;
		default: return GL_COLOR_ATTACHMENT0;
		}
	}

	GLenum ToOpenGLRenderBufferInternalType(RenderBufferInternalType internalType)
	{
		switch (internalType)
		{
		case RenderBufferInternalType::DEPTH: return GL_DEPTH_COMPONENT;
		case RenderBufferInternalType::DEPTH_16: return GL_DEPTH_COMPONENT16;
		case RenderBufferInternalType::DEPTH_24: return GL_DEPTH_COMPONENT24;
		case RenderBufferInternalType::DEPTH_32: return GL_DEPTH_COMPONENT32;
		case RenderBufferInternalType::DEPTH_32F: return GL_DEPTH_COMPONENT32F;
		case RenderBufferInternalType::DEPTH_STENCIL: return GL_DEPTH_STENCIL;
		case RenderBufferInternalType::DEPTH24_STENCIL8: return GL_DEPTH24_STENCIL8;
		case RenderBufferInternalType::RED: return GL_RED;
		case RenderBufferInternalType::RG: return GL_RG;
		case RenderBufferInternalType::RGB: return GL_RGB;
		case RenderBufferInternalType::RGB16F: return GL_RGB16F;
		case RenderBufferInternalType::RGB32F: return GL_RGB32F;
		case RenderBufferInternalType::RGBA: return GL_RGBA;
		case RenderBufferInternalType::RGBA16F: return GL_RGBA16F;
		case RenderBufferInternalType::RGBA32F: return GL_RGBA32F;
		default: return GL_RGB;
		}
	}

	GLenum ToOpenGLBlitFilter(GraphicsBlitFilter filter)
	{
		switch (filter)
		{
		case GraphicsBlitFilter::Nearest: return GL_NEAREST;
		default: return GL_NEAREST;
		}
	}

	std::string GetOpenGLInfoLog(GEuint objectId, bool isShader)
	{
		GLint maxLength = 0;
		if (isShader)
		{
			glGetShaderiv(objectId, GL_INFO_LOG_LENGTH, &maxLength);
		}
		else
		{
			glGetProgramiv(objectId, GL_INFO_LOG_LENGTH, &maxLength);
		}

		if (maxLength <= 0)
		{
			return "";
		}

		std::string logMessage(static_cast<size_t>(maxLength), '\0');
		GLsizei writtenLength = 0;
		if (isShader)
		{
			glGetShaderInfoLog(objectId, maxLength, &writtenLength, logMessage.data());
		}
		else
		{
			glGetProgramInfoLog(objectId, maxLength, &writtenLength, logMessage.data());
		}
		logMessage.resize(static_cast<size_t>(writtenLength));
		return logMessage;
	}
}

bool OpenGLGraphicsAPI::Initialize(GraphicsAPIProcAddressFunction procAddressFunction)
{
	return gladLoadGLLoader(reinterpret_cast<GLADloadproc>(procAddressFunction)) != 0;
}

void OpenGLGraphicsAPI::Shutdown()
{
}

void OpenGLGraphicsAPI::CheckErrors(const char* errorMessage) const
{
	const GLenum errorValue = glGetError();
	if (errorValue == GL_NO_ERROR)
	{
		return;
	}

	std::string errorString = "NO_ERROR";
	switch (errorValue)
	{
	case GL_INVALID_ENUM: errorString = "INVALID_ENUM"; break;
	case GL_INVALID_VALUE: errorString = "INVALID_VALUE"; break;
	case GL_INVALID_OPERATION: errorString = "INVALID_OPERATION"; break;
	case GL_STACK_OVERFLOW: errorString = "STACK_OVERFLOW"; break;
	case GL_STACK_UNDERFLOW: errorString = "STACK_UNDERFLOW"; break;
	case GL_OUT_OF_MEMORY: errorString = "OUT_OF_MEMORY"; break;
	case GL_INVALID_FRAMEBUFFER_OPERATION: errorString = "INVALID_FRAMEBUFFER_OPERATION"; break;
	default: errorString = "UNKNOWN"; break;
	}

	GOKNAR_CORE_FATAL("%s: Error Value: %s\n", errorMessage, errorString.c_str());
	DEBUG_BREAK();
	exit(EXIT_FAILURE);
}

void OpenGLGraphicsAPI::SetViewport(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}

void OpenGLGraphicsAPI::ClearColor(float red, float green, float blue, float alpha)
{
	glClearColor(red, green, blue, alpha);
}

void OpenGLGraphicsAPI::Clear(GraphicsClearBufferFlags buffers)
{
	glClear(ToOpenGLClearFlags(buffers));
}

void OpenGLGraphicsAPI::SetCapabilityEnabled(GraphicsCapability capability, bool enabled)
{
	if (enabled)
	{
		glEnable(ToOpenGLCapability(capability));
	}
	else
	{
		glDisable(ToOpenGLCapability(capability));
	}
}

void OpenGLGraphicsAPI::SetDepthMask(bool enabled)
{
	glDepthMask(enabled ? GL_TRUE : GL_FALSE);
}

void OpenGLGraphicsAPI::SetDepthFunction(GraphicsDepthFunction depthFunction)
{
	glDepthFunc(ToOpenGLDepthFunction(depthFunction));
}

void OpenGLGraphicsAPI::SetFrontFace(GraphicsFrontFace frontFace)
{
	glFrontFace(ToOpenGLFrontFace(frontFace));
}

void OpenGLGraphicsAPI::SetBlendFunction(GraphicsBlendFactor sourceFactor, GraphicsBlendFactor destinationFactor)
{
	glBlendFunc(ToOpenGLBlendFactor(sourceFactor), ToOpenGLBlendFactor(destinationFactor));
}

void OpenGLGraphicsAPI::SetColorMask(bool red, bool green, bool blue, bool alpha)
{
	glColorMask(red ? GL_TRUE : GL_FALSE, green ? GL_TRUE : GL_FALSE, blue ? GL_TRUE : GL_FALSE, alpha ? GL_TRUE : GL_FALSE);
}

GEuint OpenGLGraphicsAPI::CreateBuffer()
{
	GLuint bufferId = 0;
	glGenBuffers(1, &bufferId);
	return bufferId;
}

void OpenGLGraphicsAPI::DeleteBuffer(GEuint bufferId)
{
	glDeleteBuffers(1, &bufferId);
}

void OpenGLGraphicsAPI::BindBuffer(GraphicsBufferTarget target, GEuint bufferId)
{
	glBindBuffer(ToOpenGLBufferTarget(target), bufferId);
}

void OpenGLGraphicsAPI::BindBufferBase(GraphicsBufferTarget target, GEuint bindingIndex, GEuint bufferId)
{
	glBindBufferBase(ToOpenGLBufferTarget(target), bindingIndex, bufferId);
}

void OpenGLGraphicsAPI::BufferData(GraphicsBufferTarget target, GEsizeiptr size, const void* data, GraphicsBufferUsage usage)
{
	glBufferData(ToOpenGLBufferTarget(target), static_cast<GLsizeiptr>(size), data, ToOpenGLBufferUsage(usage));
}

void OpenGLGraphicsAPI::BufferSubData(GraphicsBufferTarget target, GEintptr offset, GEsizeiptr size, const void* data)
{
	glBufferSubData(ToOpenGLBufferTarget(target), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
}

void OpenGLGraphicsAPI::NamedBufferSubData(GEuint bufferId, GEintptr offset, GEsizeiptr size, const void* data)
{
	glNamedBufferSubData(bufferId, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
}

GEuint OpenGLGraphicsAPI::CreateVertexArray()
{
	GLuint vertexArrayId = 0;
	glGenVertexArrays(1, &vertexArrayId);
	return vertexArrayId;
}

void OpenGLGraphicsAPI::DeleteVertexArray(GEuint vertexArrayId)
{
	glDeleteVertexArrays(1, &vertexArrayId);
}

void OpenGLGraphicsAPI::BindVertexArray(GEuint vertexArrayId)
{
	glBindVertexArray(vertexArrayId);
}

void OpenGLGraphicsAPI::EnableVertexAttribArray(GEuint index)
{
	glEnableVertexAttribArray(index);
}

void OpenGLGraphicsAPI::DisableVertexAttribArray(GEuint index)
{
	glDisableVertexAttribArray(index);
}

void OpenGLGraphicsAPI::SetVertexAttribPointer(GEuint index, GEint size, GraphicsDataType type, bool normalized, GEsizei stride, const void* pointer)
{
	glVertexAttribPointer(index, size, ToOpenGLDataType(type), normalized ? GL_TRUE : GL_FALSE, stride, pointer);
}

void OpenGLGraphicsAPI::SetVertexAttribIPointer(GEuint index, GEint size, GraphicsDataType type, GEsizei stride, const void* pointer)
{
	glVertexAttribIPointer(index, size, ToOpenGLDataType(type), stride, pointer);
}

void OpenGLGraphicsAPI::SetVertexAttribDivisor(GEuint index, GEuint divisor)
{
	glVertexAttribDivisor(index, divisor);
}

void OpenGLGraphicsAPI::DrawElementsBaseVertex(GraphicsPrimitive primitive, GEsizei count, GraphicsDataType indexType, const void* indices, GEint baseVertex)
{
	glDrawElementsBaseVertex(ToOpenGLPrimitive(primitive), count, ToOpenGLDataType(indexType), indices, baseVertex);
}

void OpenGLGraphicsAPI::DrawElementsInstancedBaseVertex(GraphicsPrimitive primitive, GEsizei count, GraphicsDataType indexType, const void* indices, GEsizei instanceCount, GEint baseVertex)
{
	glDrawElementsInstancedBaseVertex(ToOpenGLPrimitive(primitive), count, ToOpenGLDataType(indexType), indices, instanceCount, baseVertex);
}

void OpenGLGraphicsAPI::DrawElementsIndirect(GraphicsPrimitive primitive, GraphicsDataType indexType, const void* indirect)
{
	glDrawElementsIndirect(ToOpenGLPrimitive(primitive), ToOpenGLDataType(indexType), indirect);
}

void OpenGLGraphicsAPI::DrawArraysIndirect(GraphicsPrimitive primitive, const void* indirect)
{
	glDrawArraysIndirect(ToOpenGLPrimitive(primitive), indirect);
}

GEuint OpenGLGraphicsAPI::CreateProgram()
{
	return glCreateProgram();
}

void OpenGLGraphicsAPI::DeleteProgram(GEuint programId)
{
	glDeleteProgram(programId);
}

void OpenGLGraphicsAPI::UseProgram(GEuint programId)
{
	glUseProgram(programId);
}

GEuint OpenGLGraphicsAPI::CreateShader(GraphicsShaderStage shaderStage)
{
	return glCreateShader(ToOpenGLShaderStage(shaderStage));
}

void OpenGLGraphicsAPI::DeleteShader(GEuint shaderId)
{
	glDeleteShader(shaderId);
}

void OpenGLGraphicsAPI::SetShaderSource(GEuint shaderId, GEsizei count, const GEchar** sources)
{
	glShaderSource(shaderId, count, sources, nullptr);
}

void OpenGLGraphicsAPI::CompileShader(GEuint shaderId)
{
	glCompileShader(shaderId);
}

bool OpenGLGraphicsAPI::GetShaderCompileStatus(GEuint shaderId) const
{
	GLint isCompiled = GL_FALSE;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled);
	return isCompiled != GL_FALSE;
}

std::string OpenGLGraphicsAPI::GetShaderInfoLog(GEuint shaderId) const
{
	return GetOpenGLInfoLog(shaderId, true);
}

void OpenGLGraphicsAPI::AttachShader(GEuint programId, GEuint shaderId)
{
	glAttachShader(programId, shaderId);
}

void OpenGLGraphicsAPI::DetachShader(GEuint programId, GEuint shaderId)
{
	glDetachShader(programId, shaderId);
}

void OpenGLGraphicsAPI::LinkProgram(GEuint programId)
{
	glLinkProgram(programId);
}

bool OpenGLGraphicsAPI::GetProgramLinkStatus(GEuint programId) const
{
	GLint isLinked = GL_FALSE;
	glGetProgramiv(programId, GL_LINK_STATUS, &isLinked);
	return isLinked != GL_FALSE;
}

std::string OpenGLGraphicsAPI::GetProgramInfoLog(GEuint programId) const
{
	return GetOpenGLInfoLog(programId, false);
}

GEint OpenGLGraphicsAPI::GetUniformLocation(GEuint programId, const char* name) const
{
	return glGetUniformLocation(programId, name);
}

void OpenGLGraphicsAPI::SetUniform1i(GEint location, GEint value)
{
	glUniform1i(location, value);
}

void OpenGLGraphicsAPI::SetUniform1iv(GEint location, GEsizei count, const GEint* values)
{
	glUniform1iv(location, count, values);
}

void OpenGLGraphicsAPI::SetUniform1f(GEint location, GEfloat value)
{
	glUniform1f(location, value);
}

void OpenGLGraphicsAPI::SetUniform1fv(GEint location, GEsizei count, const GEfloat* values)
{
	glUniform1fv(location, count, values);
}

void OpenGLGraphicsAPI::SetUniform2fv(GEint location, GEsizei count, const GEfloat* values)
{
	glUniform2fv(location, count, values);
}

void OpenGLGraphicsAPI::SetUniform3fv(GEint location, GEsizei count, const GEfloat* values)
{
	glUniform3fv(location, count, values);
}

void OpenGLGraphicsAPI::SetUniform4fv(GEint location, GEsizei count, const GEfloat* values)
{
	glUniform4fv(location, count, values);
}

void OpenGLGraphicsAPI::SetUniformMatrix4fv(GEint location, GEsizei count, bool transpose, const GEfloat* values)
{
	glUniformMatrix4fv(location, count, transpose ? GL_TRUE : GL_FALSE, values);
}

void OpenGLGraphicsAPI::DispatchCompute(GEuint groupCountX, GEuint groupCountY, GEuint groupCountZ)
{
	glDispatchCompute(groupCountX, groupCountY, groupCountZ);
}

void OpenGLGraphicsAPI::MemoryBarrier(GraphicsMemoryBarrierFlags barrierFlags)
{
	glMemoryBarrier(ToOpenGLMemoryBarrierFlags(barrierFlags));
}

void OpenGLGraphicsAPI::GetBooleanv(GraphicsParameterName pname, GEboolean* data)
{
	glGetBooleanv(ToOpenGLParameterName(pname), data);
}

void OpenGLGraphicsAPI::GetDoublev(GraphicsParameterName pname, GEdouble* data)
{
	glGetDoublev(ToOpenGLParameterName(pname), data);
}

void OpenGLGraphicsAPI::GetFloatv(GraphicsParameterName pname, GEfloat* data)
{
	glGetFloatv(ToOpenGLParameterName(pname), data);
}

void OpenGLGraphicsAPI::GetIntegerv(GraphicsParameterName pname, GEint* data)
{
	glGetIntegerv(ToOpenGLParameterName(pname), data);
}

void OpenGLGraphicsAPI::GetInteger64v(GraphicsParameterName pname, GEint64* data)
{
	glGetInteger64v(ToOpenGLParameterName(pname), data);
}

void OpenGLGraphicsAPI::GetBooleani_v(GraphicsParameterTarget target, GEuint index, GEboolean* data)
{
	glGetBooleani_v(ToOpenGLParameterTarget(target), index, data);
}

void OpenGLGraphicsAPI::GetIntegeri_v(GraphicsParameterTarget target, GEuint index, GEint* data)
{
	glGetIntegeri_v(ToOpenGLParameterTarget(target), index, data);
}

void OpenGLGraphicsAPI::GetFloati_v(GraphicsParameterTarget target, GEuint index, GEfloat* data)
{
	glGetFloati_v(ToOpenGLParameterTarget(target), index, data);
}

void OpenGLGraphicsAPI::GetDoublei_v(GraphicsParameterTarget target, GEuint index, GEdouble* data)
{
	glGetDoublei_v(ToOpenGLParameterTarget(target), index, data);
}

void OpenGLGraphicsAPI::GetInteger64i_v(GraphicsParameterTarget target, GEuint index, GEint64* data)
{
	glGetInteger64i_v(ToOpenGLParameterTarget(target), index, data);
}

GEuint OpenGLGraphicsAPI::CreateTexture()
{
	GLuint textureId = 0;
	glGenTextures(1, &textureId);
	return textureId;
}

void OpenGLGraphicsAPI::DeleteTexture(GEuint textureId)
{
	glDeleteTextures(1, &textureId);
}

void OpenGLGraphicsAPI::ActivateTextureUnit(GEuint textureUnit)
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
}

void OpenGLGraphicsAPI::BindTexture(TextureBindTarget target, GEuint textureId)
{
	glBindTexture(ToOpenGLTextureBindTarget(target), textureId);
}

void OpenGLGraphicsAPI::BindImageTexture(GEuint imageUnit, GEuint textureId, GEint level, bool layered, GEint layer, TextureImageAccess access, TextureInternalFormat format)
{
	glBindImageTexture(imageUnit, textureId, level, layered ? GL_TRUE : GL_FALSE, layer, ToOpenGLTextureImageAccess(access), ToOpenGLTextureInternalFormat(format));
}

void OpenGLGraphicsAPI::PixelStore(GraphicsPixelStoreParameter parameter, GEint value)
{
	glPixelStorei(ToOpenGLPixelStoreParameter(parameter), value);
}

void OpenGLGraphicsAPI::SetTextureImage2D(TextureImageTarget target, GEint targetOffset, GEint level, TextureInternalFormat internalFormat, GEsizei width, GEsizei height, GEint border, TextureFormat format, TextureType type, const void* data)
{
	glTexImage2D(ToOpenGLTextureImageTarget(target) + targetOffset, level, ToOpenGLTextureInternalFormat(internalFormat), width, height, border, ToOpenGLTextureFormat(format), ToOpenGLTextureType(type), data);
}

void OpenGLGraphicsAPI::SetTextureImage3D(TextureBindTarget target, GEint level, TextureInternalFormat internalFormat, GEsizei width, GEsizei height, GEsizei depth, GEint border, TextureFormat format, TextureType type, const void* data)
{
	glTexImage3D(ToOpenGLTextureBindTarget(target), level, ToOpenGLTextureInternalFormat(internalFormat), width, height, depth, border, ToOpenGLTextureFormat(format), ToOpenGLTextureType(type), data);
}

void OpenGLGraphicsAPI::SetTextureCompareMode(TextureBindTarget target, TextureCompareMode compareMode)
{
	glTexParameteri(ToOpenGLTextureBindTarget(target), GL_TEXTURE_COMPARE_MODE, ToOpenGLTextureCompareMode(compareMode));
}

void OpenGLGraphicsAPI::SetTextureCompareFunc(TextureBindTarget target, TextureCompareFunc compareFunc)
{
	glTexParameteri(ToOpenGLTextureBindTarget(target), GL_TEXTURE_COMPARE_FUNC, ToOpenGLTextureCompareFunc(compareFunc));
}

void OpenGLGraphicsAPI::SetTextureMinFilter(TextureBindTarget target, TextureMinFilter minFilter)
{
	glTexParameteri(ToOpenGLTextureBindTarget(target), GL_TEXTURE_MIN_FILTER, ToOpenGLTextureMinFilter(minFilter));
}

void OpenGLGraphicsAPI::SetTextureMagFilter(TextureBindTarget target, TextureMagFilter magFilter)
{
	glTexParameteri(ToOpenGLTextureBindTarget(target), GL_TEXTURE_MAG_FILTER, ToOpenGLTextureMagFilter(magFilter));
}

void OpenGLGraphicsAPI::SetTextureWrappingS(TextureBindTarget target, TextureWrapping wrapping)
{
	glTexParameteri(ToOpenGLTextureBindTarget(target), GL_TEXTURE_WRAP_S, ToOpenGLTextureWrapping(wrapping));
}

void OpenGLGraphicsAPI::SetTextureWrappingT(TextureBindTarget target, TextureWrapping wrapping)
{
	glTexParameteri(ToOpenGLTextureBindTarget(target), GL_TEXTURE_WRAP_T, ToOpenGLTextureWrapping(wrapping));
}

void OpenGLGraphicsAPI::SetTextureWrappingR(TextureBindTarget target, TextureWrapping wrapping)
{
	glTexParameteri(ToOpenGLTextureBindTarget(target), GL_TEXTURE_WRAP_R, ToOpenGLTextureWrapping(wrapping));
}

void OpenGLGraphicsAPI::GenerateMipmap(TextureBindTarget target)
{
	glGenerateMipmap(ToOpenGLTextureBindTarget(target));
}

void OpenGLGraphicsAPI::ReadPixels(int x, int y, int width, int height, TextureFormat format, TextureType type, void* data)
{
	glReadPixels(x, y, width, height, ToOpenGLTextureFormat(format), ToOpenGLTextureType(type), data);
}

GEuint OpenGLGraphicsAPI::CreateFrameBuffer()
{
	GLuint frameBufferId = 0;
	glGenFramebuffers(1, &frameBufferId);
	return frameBufferId;
}

void OpenGLGraphicsAPI::DeleteFrameBuffer(GEuint frameBufferId)
{
	glDeleteFramebuffers(1, &frameBufferId);
}

void OpenGLGraphicsAPI::BindFrameBuffer(FrameBufferBindTarget target, GEuint frameBufferId)
{
	glBindFramebuffer(ToOpenGLFrameBufferBindTarget(target), frameBufferId);
}

void OpenGLGraphicsAPI::DrawBufferNone()
{
	glDrawBuffer(GL_NONE);
}

void OpenGLGraphicsAPI::ReadBufferNone()
{
	glReadBuffer(GL_NONE);
}

void OpenGLGraphicsAPI::ReadBuffer(FrameBufferAttachment attachment)
{
	glReadBuffer(ToOpenGLFrameBufferAttachment(attachment));
}

void OpenGLGraphicsAPI::DrawBuffers(const std::vector<FrameBufferAttachment>& attachments)
{
	std::vector<GLenum> openGLAttachments;
	openGLAttachments.reserve(attachments.size());
	for (FrameBufferAttachment attachment : attachments)
	{
		openGLAttachments.push_back(ToOpenGLFrameBufferAttachment(attachment));
	}

	glDrawBuffers(static_cast<GLsizei>(openGLAttachments.size()), openGLAttachments.data());
}

void OpenGLGraphicsAPI::AttachTextureToFrameBuffer(FrameBufferBindTarget target, FrameBufferAttachment attachment, GEuint textureId, GEint level)
{
	glFramebufferTexture(ToOpenGLFrameBufferBindTarget(target), ToOpenGLFrameBufferAttachment(attachment), textureId, level);
}

void OpenGLGraphicsAPI::AttachTexture2DToFrameBuffer(FrameBufferBindTarget target, FrameBufferAttachment attachment, TextureImageTarget textureTarget, GEuint textureId, GEint level)
{
	glFramebufferTexture2D(ToOpenGLFrameBufferBindTarget(target), ToOpenGLFrameBufferAttachment(attachment), ToOpenGLTextureImageTarget(textureTarget), textureId, level);
}

bool OpenGLGraphicsAPI::CheckFrameBufferStatus(FrameBufferBindTarget target, std::string* outStatusName) const
{
	const GLenum status = glCheckFramebufferStatus(ToOpenGLFrameBufferBindTarget(target));
	const char* statusName = "UNKNOWN";
	switch (status)
	{
	case GL_FRAMEBUFFER_UNDEFINED: statusName = "GL_FRAMEBUFFER_UNDEFINED"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: statusName = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: statusName = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: statusName = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: statusName = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
	case GL_FRAMEBUFFER_UNSUPPORTED: statusName = "GL_FRAMEBUFFER_UNSUPPORTED"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: statusName = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: statusName = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"; break;
	case GL_FRAMEBUFFER_COMPLETE: statusName = "GL_FRAMEBUFFER_COMPLETE"; break;
	default: break;
	}

	if (outStatusName)
	{
		*outStatusName = statusName;
	}

	return status == GL_FRAMEBUFFER_COMPLETE;
}

void OpenGLGraphicsAPI::BlitFrameBuffer(int sourceX0, int sourceY0, int sourceX1, int sourceY1, int destinationX0, int destinationY0, int destinationX1, int destinationY1, GraphicsClearBufferFlags mask, GraphicsBlitFilter filter)
{
	glBlitFramebuffer(sourceX0, sourceY0, sourceX1, sourceY1, destinationX0, destinationY0, destinationX1, destinationY1, ToOpenGLClearFlags(mask), ToOpenGLBlitFilter(filter));
}

GEuint OpenGLGraphicsAPI::CreateRenderBuffer()
{
	GLuint renderBufferId = 0;
	glGenRenderbuffers(1, &renderBufferId);
	return renderBufferId;
}

void OpenGLGraphicsAPI::DeleteRenderBuffer(GEuint renderBufferId)
{
	glDeleteRenderbuffers(1, &renderBufferId);
}

void OpenGLGraphicsAPI::BindRenderBuffer(RenderBufferBindTarget target, GEuint renderBufferId)
{
	glBindRenderbuffer(ToOpenGLRenderBufferBindTarget(target), renderBufferId);
}

void OpenGLGraphicsAPI::RenderBufferStorage(RenderBufferBindTarget target, RenderBufferInternalType internalType, GEsizei width, GEsizei height)
{
	glRenderbufferStorage(ToOpenGLRenderBufferBindTarget(target), ToOpenGLRenderBufferInternalType(internalType), width, height);
}

void OpenGLGraphicsAPI::AttachRenderBufferToFrameBuffer(FrameBufferBindTarget frameBufferTarget, RenderBufferAttachment attachment, RenderBufferBindTarget renderBufferTarget, GEuint renderBufferId)
{
	glFramebufferRenderbuffer(ToOpenGLFrameBufferBindTarget(frameBufferTarget), ToOpenGLRenderBufferAttachment(attachment), ToOpenGLRenderBufferBindTarget(renderBufferTarget), renderBufferId);
}

GLenum OpenGLGraphicsAPI::ToOpenGLParameterName(GraphicsParameterName parameter)
{
	switch (parameter)
	{
	case GraphicsParameterName::MaxCombinedTextureImageUnits:
		return GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS;
	}

	return GL_NONE;
}

GLenum OpenGLGraphicsAPI::ToOpenGLParameterTarget(GraphicsParameterTarget parameter)
{
	switch (parameter)
	{

	}

	return GL_NONE;
}