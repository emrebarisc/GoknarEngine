#include "pch.h"

#include "ImGuiEditor.h"

#include "imgui.h"
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "ImGuiImplOpenGL3.h"

#include "Goknar/Engine.h"
#include "Goknar/WindowManager.h"
#include "Goknar/InputManager.h"

ImGuiEditor::ImGuiEditor()
{

}

ImGuiEditor::~ImGuiEditor()
{
	
}

void ImGuiEditor::Init()
{
	engine->GetInputManager()->AddCursorDelegate(std::bind(&ImGuiEditor::OnCursorMove, this, std::placeholders::_1, std::placeholders::_2));

	engine->GetInputManager()->AddMouseInputDelegate(GLFW_MOUSE_BUTTON_1, INPUT_ACTION::G_PRESS, std::bind(&ImGuiEditor::OnLeftClickPressed, this));
	engine->GetInputManager()->AddMouseInputDelegate(GLFW_MOUSE_BUTTON_1, INPUT_ACTION::G_RELEASE, std::bind(&ImGuiEditor::OnLeftClickRelease, this));

	windowManager_ = engine->GetWindowManager();
	const Vector2i windowSize = windowManager_->GetWindowSize();
	
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(windowSize.x, windowSize.y);
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

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

	ImGui_ImplOpenGL3_Init("#version 410 core");
}

void ImGuiEditor::Tick(float deltaTime)
{
	//glClearColor(0, 0, 0, 1);
	//glClear(GL_COLOR_BUFFER_BIT);

	const Vector2i windowSize = windowManager_->GetWindowSize();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(windowSize.x, windowSize.y);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	 
	static bool showDemo = true;
	ImGui::ShowDemoWindow(&showDemo);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	//engine->GetWindowManager()->Update();
}

void ImGuiEditor::OnCursorMove(int xPosition, int yPosition)
{
	ImGuiIO &io = ImGui::GetIO();
	io.MousePos.x = xPosition;
	io.MousePos.y = yPosition;
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
