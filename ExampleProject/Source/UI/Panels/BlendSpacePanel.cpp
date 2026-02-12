#include "BlendSpacePanel.h"

#include "Goknar/Model/SkeletalMeshInstance.h"

#include "UI/HUD.h"
#include "UI/UIContext.h"

BlendSpacePanel::BlendSpacePanel(HUD* hud) : 
	IPanel("BlendSpace", hud)
{
}

BlendSpacePanel::~BlendSpacePanel()
{
}

void BlendSpacePanel::Init()
{
}

void BlendSpacePanel::Draw()
{
	UIContext* context = UIContext::Get();

	float barLength = 200.f;

	ImGui::SetNextWindowPos({ (context->windowSize.x - barLength) * 0.5f, 10.f } );

	ImGui::Begin(
		title_.c_str(), 
		&isOpen_, 
		ImGuiWindowFlags_NoTitleBar | 
		ImGuiWindowFlags_NoScrollbar | 
		ImGuiWindowFlags_NoScrollWithMouse | 
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::SliderFloat("BlendSpaceValue", &sliderValue, 0, 1.f);

	SkeletalMeshInstance::blendValue = sliderValue;

	ImGui::End();
}
