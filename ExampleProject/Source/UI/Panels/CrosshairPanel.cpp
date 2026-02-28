#include "CrosshairPanel.h"

#include "Goknar/Engine.h"

#include "UI/HUD.h"
#include "UI/UIContext.h"
#include "UI/UIUtils.h"

#include "Game.h"
#include "GameState.h"

CrosshairPanel::CrosshairPanel(HUD* hud) : 
	IPanel("Crosshair", hud)
{
    isOpen_ = false;

    onGameResumedDelegate_ = Delegate<void()>::Create<CrosshairPanel, &CrosshairPanel::SetOpen>(this);
    onGamePausedDelegate_ = Delegate<void()>::Create<CrosshairPanel, &CrosshairPanel::SetClosed>(this);
}

CrosshairPanel::~CrosshairPanel()
{
    Game* game = (Game*)engine->GetApplication();
    GameState* gameState = game->GetGameState();

    gameState->OnGamePausedDelegate -= onGamePausedDelegate_;
    gameState->OnGameResumedDelegate -= onGameResumedDelegate_;
}

void CrosshairPanel::Init()
{
    Game* game = (Game*)engine->GetApplication();
    GameState* gameState = game->GetGameState();

    gameState->OnGamePausedDelegate += onGamePausedDelegate_;
    gameState->OnGameResumedDelegate += onGameResumedDelegate_;
}

void CrosshairPanel::Draw()
{
    UIContext* context = UIContext::Get();

    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGui::SetNextWindowSize(UIUtils::ToImVec2(context->windowSize));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("CrosshairOverlay", &isOpen_,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoBackground);

    ImVec2 screenCenter = ImVec2(context->windowSize.x * 0.5f, context->windowSize.y * 0.5f);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const float lineLength = 10.f;
    const float gap = 4.f;
    const float thickness = 2.f;
    const ImU32 foreground = IM_COL32(255, 255, 255, 255);
    const ImU32 background = IM_COL32(0, 0, 0, 200);

    auto DrawCrosshairLine = [&](ImVec2 start, ImVec2 end) {
        drawList->AddLine(start, end, background, thickness + 2.f);
        drawList->AddLine(start, end, foreground, thickness);
        };

    DrawCrosshairLine(ImVec2(screenCenter.x, screenCenter.y - gap),
        ImVec2(screenCenter.x, screenCenter.y - gap - lineLength));

    DrawCrosshairLine(ImVec2(screenCenter.x, screenCenter.y + gap),
        ImVec2(screenCenter.x, screenCenter.y + gap + lineLength));

    DrawCrosshairLine(ImVec2(screenCenter.x - gap, screenCenter.y),
        ImVec2(screenCenter.x - gap - lineLength, screenCenter.y));

    DrawCrosshairLine(ImVec2(screenCenter.x + gap, screenCenter.y),
        ImVec2(screenCenter.x + gap + lineLength, screenCenter.y));

    ImGui::End();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
}