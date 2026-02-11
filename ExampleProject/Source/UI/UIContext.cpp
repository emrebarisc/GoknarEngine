#include "UIContext.h"

#include "imgui.h"

UIContext::UIContext()
{
	imguiContext_ = ImGui::CreateContext();
}

UIContext::~UIContext()
{
}

void UIContext::Init()
{
}
