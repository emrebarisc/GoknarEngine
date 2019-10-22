#include "pch.h"

#include "ImGuiEditor.h"

#include "imgui.h"
#include "GLFW/glfw3.h"
#include "ImGuiOpenGL.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/WindowManager.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Log.h"

ImGuiEditor::ImGuiEditor() :
	showAbout_(false),
	showLog_(false)
{
}

ImGuiEditor::~ImGuiEditor()
{
	
}

void ImGuiEditor::Init()
{
	engine->GetInputManager()->AddKeyboardListener(std::bind(&ImGuiEditor::OnKeyboardEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

	engine->GetInputManager()->AddCursorDelegate(std::bind(&ImGuiEditor::OnCursorMove, this, std::placeholders::_1, std::placeholders::_2));
	engine->GetInputManager()->AddScrollDelegate(std::bind(&ImGuiEditor::OnScroll, this, std::placeholders::_1, std::placeholders::_2));

	engine->GetInputManager()->AddMouseInputDelegate(GLFW_MOUSE_BUTTON_1, INPUT_ACTION::G_PRESS, std::bind(&ImGuiEditor::OnLeftClickPressed, this));
	engine->GetInputManager()->AddMouseInputDelegate(GLFW_MOUSE_BUTTON_1, INPUT_ACTION::G_RELEASE, std::bind(&ImGuiEditor::OnLeftClickRelease, this));

	engine->GetInputManager()->AddCharDelegate(std::bind(&ImGuiEditor::OnCharPressed, this, std::placeholders::_1));

	windowManager_ = engine->GetWindowManager();
	const Vector2i windowSize = windowManager_->GetWindowSize();
	
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)windowSize.x, (float)windowSize.y);
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.KeyRepeatRate = 0.1f;

	// TODO: Implement and set Goknar Keymaps
	io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
	io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
	io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
	io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
	io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
	io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
	io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
	io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
	io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
	io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
	io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
	io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
	io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
	io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
	io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

	ImGui_Init("#version 410 core");

	GOKNAR_CORE_INFO("ImGui Editor Is Initiated.");
}

void ImGuiEditor::Tick(float deltaTime)
{
	windowSize_ = windowManager_->GetWindowSize();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)windowSize_.x, (float)windowSize_.y);

	ImGui_NewFrame();
	ImGui::NewFrame();
	 
	//static bool showDemo = true;
	//ImGui::ShowDemoWindow(&showDemo);

	ShowMainMenu();

	if(showAbout_)
	{
		ShowAbout();
	}
	
	if(showLog_)
	{
		ShowLog();
	}

	ImGui::Render();
	ImGui_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiEditor::Log(const char* logMessage)
{
	imguiLog_.AddLog(logMessage);
}

void ImGuiEditor::OnKeyboardEvent(int key, int scanCode, int action, int mod)
{
	ImGuiIO &io = ImGui::GetIO();

	switch (action)
	{
	case GLFW_PRESS:
	{
		io.KeysDown[key] = true;

		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
		break;
	}
	case GLFW_RELEASE:
	{
		io.KeysDown[key] = false;
		break;
	}
	default:
		break;
	}
}

void ImGuiEditor::OnCursorMove(double xPosition, double yPosition)
{
	ImGuiIO &io = ImGui::GetIO();
	io.MousePos.x = (float)xPosition;
	io.MousePos.y = (float)yPosition;
}

void ImGuiEditor::OnScroll(double xOffset, double yOffset)
{
	ImGuiIO &io = ImGui::GetIO();
	io.MouseWheelH += (float)xOffset;
	io.MouseWheel += (float)yOffset;
}

void ImGuiEditor::OnLeftClickPressed()
{
	ImGuiIO &io = ImGui::GetIO();
	io.MouseDown[GLFW_MOUSE_BUTTON_1] = true;
}

void ImGuiEditor::OnLeftClickRelease()
{
	ImGuiIO &io = ImGui::GetIO();
	io.MouseDown[GLFW_MOUSE_BUTTON_1] = false;
}

void ImGuiEditor::OnCharPressed(unsigned int codePoint)
{
	ImGuiIO &io = ImGui::GetIO();
	io.AddInputCharacter(codePoint);
}

void ImGuiEditor::ShowMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
			{

			}
			if (ImGui::MenuItem("Save as"))
			{

			}
			if (ImGui::MenuItem("Exit"))
			{
				engine->Exit();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem(showLog_ ? MAIN_MENU_ACTIVEABLE_ITEMS::VIEW_LOG_ACTIVE : MAIN_MENU_ACTIVEABLE_ITEMS::VIEW_LOG_PASSIVE))
			{
				showLog_ = !showLog_;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
			{
				showAbout_ = !showAbout_;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void ImGuiEditor::ShowAbout()
{
	ImGui::Begin("About", &showAbout_, ImGuiWindowFlags_NoCollapse);
	ImGui::Text("Emre Baris Coskun \nemrebariscoskun@gmail.com");
	ImGui::End();
}

void ImGuiEditor::ShowLog()
{
	imguiLog_.Draw("Goknar Log", &showLog_);
}