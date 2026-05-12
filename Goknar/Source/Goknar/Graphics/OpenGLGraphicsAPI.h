#ifndef __OPENGLGRAPHICSAPI_H__
#define __OPENGLGRAPHICSAPI_H__

#include "IGraphicsAPI.h"

class GOKNAR_API OpenGLGraphicsAPI : public IGraphicsAPI
{
public:
	bool Initialize(GraphicsAPIProcAddressFunction procAddressFunction) override;
	void Shutdown() override;
	void CheckErrors(const char* errorMessage) const override;

	void SetViewport(int x, int y, int width, int height) override;
	void ClearColor(float red, float green, float blue, float alpha) override;
	void Clear(GraphicsClearBufferFlags buffers) override;
	void SetCapabilityEnabled(GraphicsCapability capability, bool enabled) override;
	void SetDepthMask(bool enabled) override;
	void SetDepthFunction(GraphicsDepthFunction depthFunction) override;
	void SetFrontFace(GraphicsFrontFace frontFace) override;
	void SetBlendFunction(GraphicsBlendFactor sourceFactor, GraphicsBlendFactor destinationFactor) override;
	void SetColorMask(bool red, bool green, bool blue, bool alpha) override;

	GEuint CreateBuffer() override;
	void DeleteBuffer(GEuint bufferId) override;
	void BindBuffer(GraphicsBufferTarget target, GEuint bufferId) override;
	void BindBufferBase(GraphicsBufferTarget target, GEuint bindingIndex, GEuint bufferId) override;
	void BufferData(GraphicsBufferTarget target, GEsizeiptr size, const void* data, GraphicsBufferUsage usage) override;
	void BufferSubData(GraphicsBufferTarget target, GEintptr offset, GEsizeiptr size, const void* data) override;
	void NamedBufferSubData(GEuint bufferId, GEintptr offset, GEsizeiptr size, const void* data) override;

	GEuint CreateVertexArray() override;
	void DeleteVertexArray(GEuint vertexArrayId) override;
	void BindVertexArray(GEuint vertexArrayId) override;
	void EnableVertexAttribArray(GEuint index) override;
	void DisableVertexAttribArray(GEuint index) override;
	void SetVertexAttribPointer(GEuint index, GEint size, GraphicsDataType type, bool normalized, GEsizei stride, const void* pointer) override;
	void SetVertexAttribIPointer(GEuint index, GEint size, GraphicsDataType type, GEsizei stride, const void* pointer) override;
	void SetVertexAttribDivisor(GEuint index, GEuint divisor) override;

	void DrawElementsBaseVertex(GraphicsPrimitive primitive, GEsizei count, GraphicsDataType indexType, const void* indices, GEint baseVertex) override;
	void DrawElementsInstancedBaseVertex(GraphicsPrimitive primitive, GEsizei count, GraphicsDataType indexType, const void* indices, GEsizei instanceCount, GEint baseVertex) override;
	void DrawElementsIndirect(GraphicsPrimitive primitive, GraphicsDataType indexType, const void* indirect) override;
	void DrawArraysIndirect(GraphicsPrimitive primitive, const void* indirect) override;

	GEuint CreateProgram() override;
	void DeleteProgram(GEuint programId) override;
	void UseProgram(GEuint programId) override;
	GEuint CreateShader(GraphicsShaderStage shaderStage) override;
	void DeleteShader(GEuint shaderId) override;
	void SetShaderSource(GEuint shaderId, GEsizei count, const GEchar** sources) override;
	void CompileShader(GEuint shaderId) override;
	bool GetShaderCompileStatus(GEuint shaderId) const override;
	std::string GetShaderInfoLog(GEuint shaderId) const override;
	void AttachShader(GEuint programId, GEuint shaderId) override;
	void DetachShader(GEuint programId, GEuint shaderId) override;
	void LinkProgram(GEuint programId) override;
	bool GetProgramLinkStatus(GEuint programId) const override;
	std::string GetProgramInfoLog(GEuint programId) const override;
	GEint GetUniformLocation(GEuint programId, const char* name) const override;
	void SetUniform1i(GEint location, GEint value) override;
	void SetUniform1iv(GEint location, GEsizei count, const GEint* values) override;
	void SetUniform1f(GEint location, GEfloat value) override;
	void SetUniform1fv(GEint location, GEsizei count, const GEfloat* values) override;
	void SetUniform2fv(GEint location, GEsizei count, const GEfloat* values) override;
	void SetUniform3fv(GEint location, GEsizei count, const GEfloat* values) override;
	void SetUniform4fv(GEint location, GEsizei count, const GEfloat* values) override;
	void SetUniformMatrix4fv(GEint location, GEsizei count, bool transpose, const GEfloat* values) override;
	void DispatchCompute(GEuint groupCountX, GEuint groupCountY, GEuint groupCountZ) override;
	void MemoryBarrier(GraphicsMemoryBarrierFlags barrierFlags) override;

	GEuint CreateTexture() override;
	void DeleteTexture(GEuint textureId) override;
	void ActivateTextureUnit(GEuint textureUnit) override;
	void BindTexture(TextureBindTarget target, GEuint textureId) override;
	void BindImageTexture(GEuint imageUnit, GEuint textureId, GEint level, bool layered, GEint layer, TextureImageAccess access, TextureInternalFormat format) override;
	void PixelStore(GraphicsPixelStoreParameter parameter, GEint value) override;
	void SetTextureImage2D(TextureImageTarget target, GEint targetOffset, GEint level, TextureInternalFormat internalFormat, GEsizei width, GEsizei height, GEint border, TextureFormat format, TextureType type, const void* data) override;
	void SetTextureCompareMode(TextureBindTarget target, TextureCompareMode compareMode) override;
	void SetTextureCompareFunc(TextureBindTarget target, TextureCompareFunc compareFunc) override;
	void SetTextureMinFilter(TextureBindTarget target, TextureMinFilter minFilter) override;
	void SetTextureMagFilter(TextureBindTarget target, TextureMagFilter magFilter) override;
	void SetTextureWrappingS(TextureBindTarget target, TextureWrapping wrapping) override;
	void SetTextureWrappingT(TextureBindTarget target, TextureWrapping wrapping) override;
	void SetTextureWrappingR(TextureBindTarget target, TextureWrapping wrapping) override;
	void GenerateMipmap(TextureBindTarget target) override;
	void ReadPixels(int x, int y, int width, int height, TextureFormat format, TextureType type, void* data) override;

	GEuint CreateFrameBuffer() override;
	void DeleteFrameBuffer(GEuint frameBufferId) override;
	void BindFrameBuffer(FrameBufferBindTarget target, GEuint frameBufferId) override;
	void DrawBufferNone() override;
	void ReadBufferNone() override;
	void ReadBuffer(FrameBufferAttachment attachment) override;
	void DrawBuffers(const std::vector<FrameBufferAttachment>& attachments) override;
	void AttachTextureToFrameBuffer(FrameBufferBindTarget target, FrameBufferAttachment attachment, GEuint textureId, GEint level) override;
	void AttachTexture2DToFrameBuffer(FrameBufferBindTarget target, FrameBufferAttachment attachment, TextureImageTarget textureTarget, GEuint textureId, GEint level) override;
	bool CheckFrameBufferStatus(FrameBufferBindTarget target, std::string* outStatusName = nullptr) const override;
	void BlitFrameBuffer(int sourceX0, int sourceY0, int sourceX1, int sourceY1, int destinationX0, int destinationY0, int destinationX1, int destinationY1, GraphicsClearBufferFlags mask, GraphicsBlitFilter filter) override;

	GEuint CreateRenderBuffer() override;
	void DeleteRenderBuffer(GEuint renderBufferId) override;
	void BindRenderBuffer(RenderBufferBindTarget target, GEuint renderBufferId) override;
	void RenderBufferStorage(RenderBufferBindTarget target, RenderBufferInternalType internalType, GEsizei width, GEsizei height) override;
	void AttachRenderBufferToFrameBuffer(FrameBufferBindTarget frameBufferTarget, RenderBufferAttachment attachment, RenderBufferBindTarget renderBufferTarget, GEuint renderBufferId) override;
};

#endif
