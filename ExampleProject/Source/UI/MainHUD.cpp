#include "MainHUD.h"

#include <string>
#include <unordered_map>

#include "imgui.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"

#include "Goknar/Contents/Image.h"

#include "Goknar/Components/StaticMeshComponent.h"

#include "Goknar/Managers/InputManager.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Managers/WindowManager.h"

#include "Goknar/Renderer/Texture.h"

#include "UI/ImGuiOpenGL.h"
#include "UI/Panels/CrosshairPanel.h"
#include "UI/Panels/MainMenuPanel.h"

#include "UIContext.h"
#include "UIUtils.h"

#include "Game.h"
#include "GameState.h"

MainHUD::MainHUD() : HUD()
{
	engine->SetHUD(this);

	AddPanel<MainMenuPanel>();
	AddPanel<CrosshairPanel>();

	onKeyboardEventDelegate_ = Delegate<void(int, int, int, int)>::Create<MainHUD, &MainHUD::OnKeyboardEvent>(this);
	onCursorMoveDelegate_ = Delegate<void(double, double)>::Create<MainHUD, &MainHUD::OnCursorMove>(this);
	onScrollDelegate_ = Delegate<void(double, double)>::Create<MainHUD, &MainHUD::OnScroll>(this);
	onLeftClickPressedDelegate_ = Delegate<void()>::Create<MainHUD, &MainHUD::OnLeftClickPressed>(this);
	onLeftClickReleasedDelegate_ = Delegate<void()>::Create<MainHUD, &MainHUD::OnLeftClickReleased>(this);
	onCharPressedDelegate_ = Delegate<void(unsigned int)>::Create<MainHUD, &MainHUD::OnCharPressed>(this);
	onWindowSizeChangedDelegate_ = Delegate<void(int, int)>::Create<MainHUD, &MainHUD::OnWindowSizeChanged>(this);

	uiImage_ = engine->GetResourceManager()->GetContent<Image>("Textures/UITexture.png");

	engine->GetRenderer()->SetDrawOnWindow(true);

	context_ = UIContext::Get();

	GameState* gameState = dynamic_cast<Game*>(engine->GetApplication())->GetGameState();

	gameState->OnGamePausedDelegate += Delegate<void()>::Create<MainHUD, &MainHUD::OnGamePaused>(this);
	gameState->OnGameResumedDelegate += Delegate<void()>::Create<MainHUD, &MainHUD::OnGameResumed>(this);
}

MainHUD::~MainHUD()
{
	InputManager* inputManager = engine->GetInputManager();

	inputManager->RemoveKeyboardListener(onKeyboardEventDelegate_);

	inputManager->RemoveCursorDelegate(onCursorMoveDelegate_);
	inputManager->RemoveScrollDelegate(onScrollDelegate_);

	inputManager->RemoveMouseInputDelegate(MOUSE_MAP::BUTTON_1, INPUT_ACTION::G_PRESS, onLeftClickPressedDelegate_);
	inputManager->RemoveMouseInputDelegate(MOUSE_MAP::BUTTON_1, INPUT_ACTION::G_RELEASE, onLeftClickReleasedDelegate_);

	inputManager->RemoveCharDelegate(onCharPressedDelegate_);

	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::DLT, INPUT_ACTION::G_PRESS, onDeleteInputPressedDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::F, INPUT_ACTION::G_PRESS, onFocusInputPressedDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::ESCAPE, INPUT_ACTION::G_PRESS, onCancelInputPressedDelegate_);

	ImGui_DestroyFontsTexture();
	ImGui_DestroyDeviceObjects();
	ImGui_Shutdown();

	delete context_;
}

void MainHUD::PreInit()
{
	HUD::PreInit();

	InputManager* inputManager = engine->GetInputManager();

	inputManager->AddKeyboardListener(onKeyboardEventDelegate_);

	inputManager->AddCursorDelegate(onCursorMoveDelegate_);
	inputManager->AddScrollDelegate(onScrollDelegate_);

	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_1, INPUT_ACTION::G_PRESS, onLeftClickPressedDelegate_);
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_1, INPUT_ACTION::G_RELEASE, onLeftClickReleasedDelegate_);

	inputManager->AddCharDelegate(onCharPressedDelegate_);

	inputManager->AddKeyboardInputDelegate(KEY_MAP::DLT, INPUT_ACTION::G_PRESS, onDeleteInputPressedDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::F, INPUT_ACTION::G_PRESS, onFocusInputPressedDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::ESCAPE, INPUT_ACTION::G_PRESS, onCancelInputPressedDelegate_);

	context_->Init();

	Vector2i windowSize = engine->GetWindowManager()->GetWindowSize();

	OnWindowSizeChanged(windowSize.x, windowSize.y);
}

void MainHUD::Init()
{
	HUD::Init();

	std::vector<std::unique_ptr<IPanel>>::const_iterator panelIterator = panels_.cbegin();
	while (panelIterator != panels_.cend())
	{
		panelIterator->get()->Init();
		++panelIterator;
	}
}

void MainHUD::PostInit()
{
	HUD::PostInit();
}

void MainHUD::BeginGame()
{
	HUD::BeginGame();

	WindowManager* windowManager = engine->GetWindowManager();
	windowManager->AddWindowSizeCallback(onWindowSizeChangedDelegate_);

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = UIUtils::ToImVec2(windowManager->GetWindowSize());
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.KeyRepeatRate = 0.25f;

	ImGui_Init("#version 410 core");

	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;
	colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.f);
	colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.f);
}

void MainHUD::UpdateHUD()
{
	HUD::UpdateHUD();

	if (!engine->GetRenderer()->GetDrawOnWindow())
	{
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	WindowManager* windowManager = engine->GetWindowManager();
	context_->windowSize = windowManager->GetWindowSize();

	ImGui_NewFrame();
	ImGui::NewFrame();

	ImGui::StyleColorsDark();

	DrawMainHUD();

	ImGui::Render();
	ImGui_RenderDrawData(ImGui::GetDrawData());
}

void MainHUD::OnGamePaused()
{
	ShowPanel<MainMenuPanel>();
}

void MainHUD::OnGameResumed()
{
	HidePanel<MainMenuPanel>();
}

void MainHUD::DrawMainHUD()
{
	std::vector<std::unique_ptr<IPanel>>::const_iterator panelIterator = panels_.cbegin();
	while (panelIterator != panels_.cend())
	{
		IPanel* panel = panelIterator->get();

		if (panel->GetIsOpen())
		{
			panel->Draw();
		}

		++panelIterator;
	}
}

void MainHUD::OnKeyboardEvent(int key, int scanCode, int action, int)
{
	ImGuiIO& io = ImGui::GetIO();
	bool is_down = (action == GLFW_PRESS || action == GLFW_REPEAT);
	ImGuiKey imgui_key = ImGui_ImplGlfw_KeyToImGuiKey(key, scanCode);
	io.AddKeyEvent(imgui_key, is_down);
}

void MainHUD::OnCursorMove(double xPosition, double yPosition)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos.x = (float)xPosition;
	io.MousePos.y = (float)yPosition;
}

void MainHUD::OnScroll(double xOffset, double yOffset)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheelH += (float)xOffset;
	io.MouseWheel += (float)yOffset;
}

void MainHUD::OnLeftClickPressed()
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDown[GLFW_MOUSE_BUTTON_1] = true;
}

void MainHUD::OnLeftClickReleased()
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDown[GLFW_MOUSE_BUTTON_1] = false;
}

void MainHUD::OnCharPressed(unsigned int codePoint)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(codePoint);
}

void MainHUD::OnWindowSizeChanged(int width, int height)
{
	context_->windowSize = Vector2i(width, height);
	Vector2i buttonSizeVector = context_->windowSize * 0.05f;
	context_->buttonSize = Vector2i(buttonSizeVector.x, buttonSizeVector.y);

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = UIUtils::ToImVec2(context_->windowSize);
}