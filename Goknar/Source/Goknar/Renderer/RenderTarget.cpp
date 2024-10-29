#include "pch.h"

#include "RenderTarget.h"

#include "Camera.h"
#include "Engine.h"
#include "Managers/CameraManager.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Renderer.h"
#include "Renderer/Texture.h"

RenderTarget::RenderTarget()
{
	deferredRenderingData_ = new DeferredRenderingData();
}

RenderTarget::~RenderTarget()
{
	engine->GetRenderer()->RemoveRenderTarget(this);

	if (engine->GetCameraManager()->DoesCameraExist(camera_))
	{
		camera_->Destroy();
	}
	delete texture_;
	delete framebuffer_;
	delete deferredRenderingData_;
}

void RenderTarget::Init()
{
	if (!camera_)
	{
		camera_ = new Camera();
	}

	camera_->SetImageWidth(frameSize_.x);
	camera_->SetImageHeight(frameSize_.y);

	GenerateBuffers();

	deferredRenderingData_->geometryBufferData->bufferWidth = frameSize_.x;
	deferredRenderingData_->geometryBufferData->bufferHeight = frameSize_.y;
	deferredRenderingData_->PreInit();
	deferredRenderingData_->Init();

	engine->GetRenderer()->AddRenderTarget(this);
}

void RenderTarget::SetFrameSize(const Vector2& frameSize)
{
	frameSize_ = frameSize;

	camera_->SetImageWidth(frameSize_.x);
	camera_->SetImageHeight(frameSize_.y);

	deferredRenderingData_->OnViewportSizeChanged(frameSize_.x, frameSize_.y);

	delete texture_;
	delete framebuffer_;

	GenerateBuffers();
}

void RenderTarget::GenerateBuffers()
{
	texture_ = new Texture();
	texture_->SetTextureDataType(TextureDataType::DYNAMIC);
	texture_->SetTextureFormat(TextureFormat::RGB);
	texture_->SetTextureInternalFormat(TextureInternalFormat::RGB);
	texture_->SetTextureMinFilter(TextureMinFilter::NEAREST);
	texture_->SetTextureMagFilter(TextureMagFilter::NEAREST);
	texture_->SetWidth(frameSize_.x);
	texture_->SetHeight(frameSize_.y);
	texture_->SetGenerateMipmap(false);
	texture_->SetTextureType(TextureType::FLOAT);
	texture_->PreInit();
	texture_->Init();
	texture_->PostInit();

	framebuffer_ = new FrameBuffer();

	framebuffer_->AddTextureAttachment(FrameBufferAttachment::COLOR_ATTACHMENT0, texture_);

	framebuffer_->PreInit();
	framebuffer_->Init();
	framebuffer_->PostInit();
	framebuffer_->Bind();
	framebuffer_->Attach();
	framebuffer_->DrawBuffers();
	framebuffer_->Unbind();
}
