#ifndef __IGRAPHICSAPI_H__
#define __IGRAPHICSAPI_H__

#include "Goknar/Core.h"
#include "Goknar/Renderer/Types.h"

#include <string>
#include <vector>

enum class FrameBufferAttachment;
enum class FrameBufferBindTarget;
enum class RenderBufferAttachment;
enum class RenderBufferBindTarget;
enum class RenderBufferInternalType;
enum class TextureBindTarget;
enum class TextureCompareFunc;
enum class TextureCompareMode;
enum class TextureFormat;
enum class TextureImageAccess;
enum class TextureImageTarget;
enum class TextureInternalFormat;
enum class TextureMagFilter;
enum class TextureMinFilter;
enum class TextureType;
enum class TextureWrapping;

using GraphicsAPIProcAddressFunction = void* (*)(const char*);

enum class GraphicsCapability
{
	DepthTest,
	Blend,
	CullFace,
	Multisample
};

enum class GraphicsDepthFunction
{
	Lequal
};

enum class GraphicsFrontFace
{
	CounterClockwise
};

enum class GraphicsBlendFactor
{
	SourceAlpha,
	OneMinusSourceAlpha
};

enum class GraphicsClearBuffer : GEbitfield
{
	Color = 1u << 0u,
	Depth = 1u << 1u
};

using GraphicsClearBufferFlags = GEbitfield;

inline GraphicsClearBufferFlags operator|(GraphicsClearBuffer left, GraphicsClearBuffer right)
{
	return static_cast<GraphicsClearBufferFlags>(left) | static_cast<GraphicsClearBufferFlags>(right);
}

enum class GraphicsBufferTarget
{
	ArrayBuffer,
	ElementArrayBuffer,
	UniformBuffer,
	ShaderStorageBuffer,
	DrawIndirectBuffer
};

enum class GraphicsBufferUsage
{
	StaticDraw,
	DynamicDraw
};

enum class GraphicsPrimitive
{
	Triangles
};

enum class GraphicsDataType
{
	Float,
	UnsignedInt
};

enum class GraphicsShaderStage
{
	Vertex,
	Fragment,
	Geometry,
	Compute
};

enum class GraphicsMemoryBarrier : GEbitfield
{
	ShaderStorage = 1u << 0u,
	Command = 1u << 1u,
	ShaderImageAccess = 1u << 2u,
	TextureFetch = 1u << 3u
};

using GraphicsMemoryBarrierFlags = GEbitfield;

inline GraphicsMemoryBarrierFlags operator|(GraphicsMemoryBarrier left, GraphicsMemoryBarrier right)
{
	return static_cast<GraphicsMemoryBarrierFlags>(left) | static_cast<GraphicsMemoryBarrierFlags>(right);
}

inline GraphicsMemoryBarrierFlags operator|(GraphicsMemoryBarrierFlags left, GraphicsMemoryBarrier right)
{
	return left | static_cast<GraphicsMemoryBarrierFlags>(right);
}

enum class GraphicsBlitFilter
{
	Nearest
};

enum class GraphicsPixelStoreParameter
{
	UnpackAlignment
};

enum class GraphicsParameterName
{
	MaxCombinedTextureImageUnits
};

enum class GraphicsParameterTarget
{
};

class GOKNAR_API IGraphicsAPI
{
public:
	virtual ~IGraphicsAPI() = default;

	virtual bool Initialize(GraphicsAPIProcAddressFunction procAddressFunction) = 0;
	virtual void Shutdown() = 0;
	virtual void CheckErrors(const char* errorMessage) const = 0;

	virtual void SetViewport(int x, int y, int width, int height) = 0;
	virtual void ClearColor(float red, float green, float blue, float alpha) = 0;
	virtual void Clear(GraphicsClearBufferFlags buffers) = 0;
	virtual void SetCapabilityEnabled(GraphicsCapability capability, bool enabled) = 0;
	virtual void SetDepthMask(bool enabled) = 0;
	virtual void SetDepthFunction(GraphicsDepthFunction depthFunction) = 0;
	virtual void SetFrontFace(GraphicsFrontFace frontFace) = 0;
	virtual void SetBlendFunction(GraphicsBlendFactor sourceFactor, GraphicsBlendFactor destinationFactor) = 0;
	virtual void SetColorMask(bool red, bool green, bool blue, bool alpha) = 0;

	virtual GEuint CreateBuffer() = 0;
	virtual void DeleteBuffer(GEuint bufferId) = 0;
	virtual void BindBuffer(GraphicsBufferTarget target, GEuint bufferId) = 0;
	virtual void BindBufferBase(GraphicsBufferTarget target, GEuint bindingIndex, GEuint bufferId) = 0;
	virtual void BufferData(GraphicsBufferTarget target, GEsizeiptr size, const void* data, GraphicsBufferUsage usage) = 0;
	virtual void BufferSubData(GraphicsBufferTarget target, GEintptr offset, GEsizeiptr size, const void* data) = 0;
	virtual void NamedBufferSubData(GEuint bufferId, GEintptr offset, GEsizeiptr size, const void* data) = 0;

	virtual GEuint CreateVertexArray() = 0;
	virtual void DeleteVertexArray(GEuint vertexArrayId) = 0;
	virtual void BindVertexArray(GEuint vertexArrayId) = 0;
	virtual void EnableVertexAttribArray(GEuint index) = 0;
	virtual void DisableVertexAttribArray(GEuint index) = 0;
	virtual void SetVertexAttribPointer(GEuint index, GEint size, GraphicsDataType type, bool normalized, GEsizei stride, const void* pointer) = 0;
	virtual void SetVertexAttribIPointer(GEuint index, GEint size, GraphicsDataType type, GEsizei stride, const void* pointer) = 0;
	virtual void SetVertexAttribDivisor(GEuint index, GEuint divisor) = 0;

	virtual void DrawElementsBaseVertex(GraphicsPrimitive primitive, GEsizei count, GraphicsDataType indexType, const void* indices, GEint baseVertex) = 0;
	virtual void DrawElementsInstancedBaseVertex(GraphicsPrimitive primitive, GEsizei count, GraphicsDataType indexType, const void* indices, GEsizei instanceCount, GEint baseVertex) = 0;
	virtual void DrawElementsIndirect(GraphicsPrimitive primitive, GraphicsDataType indexType, const void* indirect) = 0;
	virtual void DrawArraysIndirect(GraphicsPrimitive primitive, const void* indirect) = 0;

	virtual GEuint CreateProgram() = 0;
	virtual void DeleteProgram(GEuint programId) = 0;
	virtual void UseProgram(GEuint programId) = 0;
	virtual GEuint CreateShader(GraphicsShaderStage shaderStage) = 0;
	virtual void DeleteShader(GEuint shaderId) = 0;
	virtual void SetShaderSource(GEuint shaderId, GEsizei count, const GEchar** sources) = 0;
	virtual void CompileShader(GEuint shaderId) = 0;
	virtual bool GetShaderCompileStatus(GEuint shaderId) const = 0;
	virtual std::string GetShaderInfoLog(GEuint shaderId) const = 0;
	virtual void AttachShader(GEuint programId, GEuint shaderId) = 0;
	virtual void DetachShader(GEuint programId, GEuint shaderId) = 0;
	virtual void LinkProgram(GEuint programId) = 0;
	virtual bool GetProgramLinkStatus(GEuint programId) const = 0;
	virtual std::string GetProgramInfoLog(GEuint programId) const = 0;
	virtual GEint GetUniformLocation(GEuint programId, const char* name) const = 0;
	virtual void SetUniform1i(GEint location, GEint value) = 0;
	virtual void SetUniform1iv(GEint location, GEsizei count, const GEint* values) = 0;
	virtual void SetUniform1f(GEint location, GEfloat value) = 0;
	virtual void SetUniform1fv(GEint location, GEsizei count, const GEfloat* values) = 0;
	virtual void SetUniform2fv(GEint location, GEsizei count, const GEfloat* values) = 0;
	virtual void SetUniform3fv(GEint location, GEsizei count, const GEfloat* values) = 0;
	virtual void SetUniform4fv(GEint location, GEsizei count, const GEfloat* values) = 0;
	virtual void SetUniformMatrix4fv(GEint location, GEsizei count, bool transpose, const GEfloat* values) = 0;
	virtual void DispatchCompute(GEuint groupCountX, GEuint groupCountY, GEuint groupCountZ) = 0;
	virtual void MemoryBarrier(GraphicsMemoryBarrierFlags barrierFlags) = 0;

	virtual void GetBooleanv(GraphicsParameterName pname, GEboolean* data) = 0;
	virtual void GetDoublev(GraphicsParameterName pname, GEdouble* data) = 0;
	virtual void GetFloatv(GraphicsParameterName pname, GEfloat* data) = 0;
	virtual void GetIntegerv(GraphicsParameterName pname, GEint* data) = 0;
	virtual void GetInteger64v(GraphicsParameterName pname, GEint64* data) = 0;
	virtual void GetBooleani_v(GraphicsParameterTarget target, GEuint index, GEboolean* data) = 0;
	virtual void GetIntegeri_v(GraphicsParameterTarget target, GEuint index, GEint* data) = 0;
	virtual void GetFloati_v(GraphicsParameterTarget target, GEuint index, GEfloat* data) = 0;
	virtual void GetDoublei_v(GraphicsParameterTarget target, GEuint index, GEdouble* data) = 0;
	virtual void GetInteger64i_v(GraphicsParameterTarget target, GEuint index, GEint64* data) = 0;

	virtual GEuint CreateTexture() = 0;
	virtual void DeleteTexture(GEuint textureId) = 0;
	virtual void ActivateTextureUnit(GEuint textureUnit) = 0;
	virtual void BindTexture(TextureBindTarget target, GEuint textureId) = 0;
	virtual void BindImageTexture(GEuint imageUnit, GEuint textureId, GEint level, bool layered, GEint layer, TextureImageAccess access, TextureInternalFormat format) = 0;
	virtual void PixelStore(GraphicsPixelStoreParameter parameter, GEint value) = 0;
	virtual void SetTextureImage2D(TextureImageTarget target, GEint targetOffset, GEint level, TextureInternalFormat internalFormat, GEsizei width, GEsizei height, GEint border, TextureFormat format, TextureType type, const void* data) = 0;
	virtual void SetTextureImage3D(TextureBindTarget target, GEint level, TextureInternalFormat internalFormat, GEsizei width, GEsizei height, GEsizei depth, GEint border, TextureFormat format, TextureType type, const void* data) = 0;
	virtual void SetTextureCompareMode(TextureBindTarget target, TextureCompareMode compareMode) = 0;
	virtual void SetTextureCompareFunc(TextureBindTarget target, TextureCompareFunc compareFunc) = 0;
	virtual void SetTextureMinFilter(TextureBindTarget target, TextureMinFilter minFilter) = 0;
	virtual void SetTextureMagFilter(TextureBindTarget target, TextureMagFilter magFilter) = 0;
	virtual void SetTextureWrappingS(TextureBindTarget target, TextureWrapping wrapping) = 0;
	virtual void SetTextureWrappingT(TextureBindTarget target, TextureWrapping wrapping) = 0;
	virtual void SetTextureWrappingR(TextureBindTarget target, TextureWrapping wrapping) = 0;
	virtual void GenerateMipmap(TextureBindTarget target) = 0;
	virtual void ReadPixels(int x, int y, int width, int height, TextureFormat format, TextureType type, void* data) = 0;

	virtual GEuint CreateFrameBuffer() = 0;
	virtual void DeleteFrameBuffer(GEuint frameBufferId) = 0;
	virtual void BindFrameBuffer(FrameBufferBindTarget target, GEuint frameBufferId) = 0;
	virtual void DrawBufferNone() = 0;
	virtual void ReadBufferNone() = 0;
	virtual void ReadBuffer(FrameBufferAttachment attachment) = 0;
	virtual void DrawBuffers(const std::vector<FrameBufferAttachment>& attachments) = 0;
	virtual void AttachTextureToFrameBuffer(FrameBufferBindTarget target, FrameBufferAttachment attachment, GEuint textureId, GEint level) = 0;
	virtual void AttachTexture2DToFrameBuffer(FrameBufferBindTarget target, FrameBufferAttachment attachment, TextureImageTarget textureTarget, GEuint textureId, GEint level) = 0;
	virtual bool CheckFrameBufferStatus(FrameBufferBindTarget target, std::string* outStatusName = nullptr) const = 0;
	virtual void BlitFrameBuffer(int sourceX0, int sourceY0, int sourceX1, int sourceY1, int destinationX0, int destinationY0, int destinationX1, int destinationY1, GraphicsClearBufferFlags mask, GraphicsBlitFilter filter) = 0;

	virtual GEuint CreateRenderBuffer() = 0;
	virtual void DeleteRenderBuffer(GEuint renderBufferId) = 0;
	virtual void BindRenderBuffer(RenderBufferBindTarget target, GEuint renderBufferId) = 0;
	virtual void RenderBufferStorage(RenderBufferBindTarget target, RenderBufferInternalType internalType, GEsizei width, GEsizei height) = 0;
	virtual void AttachRenderBufferToFrameBuffer(FrameBufferBindTarget frameBufferTarget, RenderBufferAttachment attachment, RenderBufferBindTarget renderBufferTarget, GEuint renderBufferId) = 0;
};

#endif
