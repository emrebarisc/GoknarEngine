#include "pch.h"

#include "CubemapRenderTarget.h"

#include "Camera.h"
#include "GoknarAssert.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Texture.h"

CubemapRenderTarget::~CubemapRenderTarget()
{
	delete depthTexture_;
}

void CubemapRenderTarget::Init()
{
	if (!camera_)
	{
		camera_ = new Camera();
		ownsCamera_ = true;
	}

	camera_->SetProjection(CameraProjection::Perspective);
	camera_->SetCameraType(CameraType::RenderTarget);
	camera_->SetImageWidth(frameSize_.x);
	camera_->SetImageHeight(frameSize_.y);

	GenerateBuffers();
}

void CubemapRenderTarget::SetFrameSize(const Vector2& frameSize)
{
	GOKNAR_ASSERT(0 < frameSize.x || 0 < frameSize.y);

	if ((frameSize_ - frameSize).Length() < EPSILON)
	{
		return;
	}

	frameSize_ = frameSize;

	if (camera_)
	{
		camera_->SetImageWidth(frameSize_.x);
		camera_->SetImageHeight(frameSize_.y);
	}

	delete texture_;
	texture_ = nullptr;

	delete depthTexture_;
	depthTexture_ = nullptr;

	delete framebuffer_;
	framebuffer_ = nullptr;

	GenerateBuffers();
}

void CubemapRenderTarget::GenerateMipmaps() const
{
	if (texture_)
	{
		texture_->GenerateMipmap();
	}
}

void CubemapRenderTarget::GenerateBuffers()
{
	framebuffer_ = new FrameBuffer();

	texture_ = new Texture();
	texture_->SetTextureDataType(TextureDataType::DYNAMIC);
	texture_->SetTextureBindTarget(TextureBindTarget::TEXTURE_CUBE_MAP);
	texture_->SetTextureImageTarget(TextureImageTarget::TEXTURE_CUBE_MAP_POSITIVE_X);
	texture_->SetTextureWrappingS(TextureWrapping::CLAMP_TO_EDGE);
	texture_->SetTextureWrappingT(TextureWrapping::CLAMP_TO_EDGE);
	texture_->SetTextureWrappingR(TextureWrapping::CLAMP_TO_EDGE);
	texture_->SetTextureFormat(TextureFormat::RGBA);
	texture_->SetTextureInternalFormat(TextureInternalFormat::RGBA16F);
	texture_->SetTextureMinFilter(TextureMinFilter::LINEAR_MIPMAP_LINEAR);
	texture_->SetTextureMagFilter(TextureMagFilter::LINEAR);
	texture_->SetTextureType(TextureType::FLOAT);
	texture_->SetWidth(frameSize_.x);
	texture_->SetHeight(frameSize_.y);
	texture_->SetGenerateMipmap(true);
	texture_->PreInit();
	texture_->Init();
	texture_->PostInit();
	framebuffer_->AddTextureAttachment(FrameBufferAttachment::COLOR_ATTACHMENT0, texture_);

	depthTexture_ = new Texture();
	depthTexture_->SetTextureDataType(TextureDataType::DYNAMIC);
	depthTexture_->SetTextureBindTarget(TextureBindTarget::TEXTURE_CUBE_MAP);
	depthTexture_->SetTextureImageTarget(TextureImageTarget::TEXTURE_CUBE_MAP_POSITIVE_X);
	depthTexture_->SetTextureWrappingS(TextureWrapping::CLAMP_TO_EDGE);
	depthTexture_->SetTextureWrappingT(TextureWrapping::CLAMP_TO_EDGE);
	depthTexture_->SetTextureWrappingR(TextureWrapping::CLAMP_TO_EDGE);
	depthTexture_->SetTextureFormat(TextureFormat::DEPTH);
	depthTexture_->SetTextureInternalFormat(TextureInternalFormat::DEPTH_24);
	depthTexture_->SetTextureMinFilter(TextureMinFilter::NEAREST);
	depthTexture_->SetTextureMagFilter(TextureMagFilter::NEAREST);
	depthTexture_->SetTextureType(TextureType::FLOAT);
	depthTexture_->SetWidth(frameSize_.x);
	depthTexture_->SetHeight(frameSize_.y);
	depthTexture_->SetGenerateMipmap(false);
	depthTexture_->PreInit();
	depthTexture_->Init();
	depthTexture_->PostInit();
	framebuffer_->AddTextureAttachment(FrameBufferAttachment::DEPTH_ATTACHMENT, depthTexture_);

	framebuffer_->PreInit();
	framebuffer_->Init();
	framebuffer_->PostInit();
	framebuffer_->Bind();
	framebuffer_->Attach();
	framebuffer_->DrawBuffers();
	framebuffer_->Unbind();
}
