#include "UIContext.h"

#include "imgui.h"

UIContext::UIContext()
{
	imguiContext_ = ImGui::CreateContext();
}

void UIContext::Destroy()
{
	delete instance_;
	instance_ = nullptr;
}

UIContext::~UIContext()
{
	if (imguiContext_)
	{
		ImGui::DestroyContext(imguiContext_);
		imguiContext_ = nullptr;
	}

	if (instance_ == this)
	{
		instance_ = nullptr;
	}
}

void UIContext::Init()
{
}
