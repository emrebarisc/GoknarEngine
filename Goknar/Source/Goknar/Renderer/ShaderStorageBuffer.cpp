#include "pch.h"

#include "ShaderStorageBuffer.h"

#include "Goknar/Engine.h"

namespace
{
	constexpr GEsizeiptr MINIMUM_STORAGE_BUFFER_CAPACITY = 16;

	IGraphicsAPI* GraphicsAPI()
	{
		return engine ? engine->GetGraphicsAPI() : nullptr;
	}

	GEsizeiptr NormalizeCapacity(GEsizeiptr capacityBytes)
	{
		return 0 < capacityBytes ? capacityBytes : MINIMUM_STORAGE_BUFFER_CAPACITY;
	}
}

ShaderStorageBuffer::~ShaderStorageBuffer()
{
	Destroy();
}

void ShaderStorageBuffer::Create(GEsizeiptr capacityBytes, const void* initialData, GraphicsBufferUsage usage)
{
	if (bufferId_ == 0u)
	{
		IGraphicsAPI* graphicsAPI = GraphicsAPI();
		if (!graphicsAPI)
		{
			return;
		}

		bufferId_ = graphicsAPI->CreateBuffer();
	}

	if (capacityBytes != 0 || initialData)
	{
		Allocate(NormalizeCapacity(capacityBytes), initialData, usage);
	}
}

void ShaderStorageBuffer::Destroy()
{
	if (bufferId_ == 0u)
	{
		return;
	}

	if (IGraphicsAPI* graphicsAPI = GraphicsAPI())
	{
		graphicsAPI->DeleteBuffer(bufferId_);
	}

	bufferId_ = 0u;
	capacityBytes_ = 0;
}

void ShaderStorageBuffer::Bind() const
{
	if (IGraphicsAPI* graphicsAPI = GraphicsAPI())
	{
		graphicsAPI->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, bufferId_);
	}
}

void ShaderStorageBuffer::BindToBindingPoint(GEuint bindingPoint) const
{
	if (IGraphicsAPI* graphicsAPI = GraphicsAPI())
	{
		graphicsAPI->BindBufferBase(GraphicsBufferTarget::ShaderStorageBuffer, bindingPoint, bufferId_);
	}
}

void ShaderStorageBuffer::Upload(const void* data, GEsizeiptr size, GraphicsBufferUsage usage, bool allowGrow)
{
	if (size <= 0)
	{
		return;
	}

	if (bufferId_ == 0u)
	{
		Create();
	}

	if (capacityBytes_ < size)
	{
		if (!allowGrow)
		{
			return;
		}

		Allocate(size, data, usage);
		return;
	}

	if (IGraphicsAPI* graphicsAPI = GraphicsAPI())
	{
		graphicsAPI->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, bufferId_);
		graphicsAPI->BufferSubData(GraphicsBufferTarget::ShaderStorageBuffer, 0, size, data);
		graphicsAPI->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, 0u);
	}
}

void ShaderStorageBuffer::Update(GEintptr offset, const void* data, GEsizeiptr size, bool allowGrow, GraphicsBufferUsage usage)
{
	if (offset < 0 || size <= 0)
	{
		return;
	}

	if (bufferId_ == 0u)
	{
		Create();
	}

	const GEsizeiptr requiredCapacity = offset + size;
	if (capacityBytes_ < requiredCapacity)
	{
		if (!allowGrow)
		{
			return;
		}

		EnsureCapacity(requiredCapacity, usage);
	}

	if (IGraphicsAPI* graphicsAPI = GraphicsAPI())
	{
		graphicsAPI->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, bufferId_);
		graphicsAPI->BufferSubData(GraphicsBufferTarget::ShaderStorageBuffer, offset, size, data);
		graphicsAPI->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, 0u);
	}
}

void ShaderStorageBuffer::EnsureCapacity(GEsizeiptr requiredCapacityBytes, GraphicsBufferUsage usage)
{
	if (requiredCapacityBytes <= capacityBytes_)
	{
		return;
	}

	GEsizeiptr newCapacity = 0 < capacityBytes_ ? capacityBytes_ : MINIMUM_STORAGE_BUFFER_CAPACITY;
	while (newCapacity < requiredCapacityBytes)
	{
		newCapacity *= 2;
	}

	Allocate(newCapacity, nullptr, usage);
}

void ShaderStorageBuffer::Unbind()
{
	if (IGraphicsAPI* graphicsAPI = GraphicsAPI())
	{
		graphicsAPI->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, 0u);
	}
}

void ShaderStorageBuffer::Allocate(GEsizeiptr capacityBytes, const void* initialData, GraphicsBufferUsage usage)
{
	if (bufferId_ == 0u)
	{
		Create();
	}

	if (IGraphicsAPI* graphicsAPI = GraphicsAPI())
	{
		capacityBytes_ = NormalizeCapacity(capacityBytes);
		graphicsAPI->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, bufferId_);
		graphicsAPI->BufferData(GraphicsBufferTarget::ShaderStorageBuffer, capacityBytes_, initialData, usage);
		graphicsAPI->BindBuffer(GraphicsBufferTarget::ShaderStorageBuffer, 0u);
	}
}
