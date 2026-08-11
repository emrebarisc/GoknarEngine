#ifndef __SHADERSTORAGEBUFFER_H__
#define __SHADERSTORAGEBUFFER_H__

#include "Goknar/Core.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/Renderer/Types.h"

class GOKNAR_API ShaderStorageBuffer
{
public:
	ShaderStorageBuffer() = default;
	~ShaderStorageBuffer();

	ShaderStorageBuffer(const ShaderStorageBuffer&) = delete;
	ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;

	void Create(GEsizeiptr capacityBytes = 0, const void* initialData = nullptr, GraphicsBufferUsage usage = GraphicsBufferUsage::DynamicDraw);
	void Destroy();

	bool IsCreated() const
	{
		return bufferId_ != 0u;
	}

	GEuint GetBufferId() const
	{
		return bufferId_;
	}

	GEsizeiptr GetCapacityBytes() const
	{
		return capacityBytes_;
	}

	void Bind() const;
	void BindToBindingPoint(GEuint bindingPoint) const;
	void Upload(const void* data, GEsizeiptr size, GraphicsBufferUsage usage = GraphicsBufferUsage::DynamicDraw, bool allowGrow = true);
	void Update(GEintptr offset, const void* data, GEsizeiptr size, bool allowGrow = false, GraphicsBufferUsage usage = GraphicsBufferUsage::DynamicDraw);
	void EnsureCapacity(GEsizeiptr requiredCapacityBytes, GraphicsBufferUsage usage = GraphicsBufferUsage::DynamicDraw);

	static void Unbind();

private:
	void Allocate(GEsizeiptr capacityBytes, const void* initialData, GraphicsBufferUsage usage);

	GEuint bufferId_{ 0u };
	GEsizeiptr capacityBytes_{ 0 };
};

#endif
