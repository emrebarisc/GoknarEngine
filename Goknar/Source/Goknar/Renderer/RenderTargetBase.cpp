#include "pch.h"

#include "RenderTargetBase.h"

#include "Camera.h"
#include "Engine.h"
#include "Managers/CameraManager.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Texture.h"

RenderTargetBase::~RenderTargetBase()
{
	if (ownsCamera_ && camera_)
	{
		camera_->Destroy();
		camera_ = nullptr;
	}

	delete texture_;
	delete framebuffer_;
}
