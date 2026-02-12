#include "MainMenuPanel.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/WindowManager.h"

#include "Game.h"
#include "GameState.h"

#include "UI/HUD.h"
#include "UI/UIContext.h"
#include "UI/UIUtils.h"

MainMenuPanel::MainMenuPanel(HUD* hud) : 
	IPanel("MainMenu", hud)
{
}

MainMenuPanel::~MainMenuPanel()
{
}

void MainMenuPanel::Init()
{
}

void MainMenuPanel::Draw()
{
	UIContext* context = UIContext::Get();

	ImGui::SetNextWindowSize(UIUtils::ToImVec2(context->windowSize));
	ImGui::SetNextWindowPos({ 0.f, 0.f } );

	ImGui::Begin(title_.c_str(), &isOpen_, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImGui::SetCursorPos(UIUtils::ToImVec2(context->windowSize * 0.5f - Vector2{25.f, 25.f}));
	if (ImGui::Button("Play", UIUtils::ToImVec2(context->buttonSize)))
	{
		dynamic_cast<Game*>(engine->GetApplication())->GetGameState()->ResumeGame();
	}

	ImGui::SetCursorPos(UIUtils::ToImVec2(context->windowSize * 0.5f - Vector2{ 25.f, -25.f })); 
	if (ImGui::Button("Quit", UIUtils::ToImVec2(context->buttonSize)))
	{
		engine->Exit();
	}

	ImGui::End();
}
