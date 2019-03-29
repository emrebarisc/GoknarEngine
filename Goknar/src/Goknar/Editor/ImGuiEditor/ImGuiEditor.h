#pragma once

#include "../Editor.h"

struct GLFWwindow;

class GOKNAR_API ImGuiEditor : public Editor
{
public:
	ImGuiEditor();
	~ImGuiEditor();

	void Init() override;
	void Tick(float deltaTime) override;

	void OnKeyboardEvent(int key, int scanCode, int action, int mod);

	void OnCursorMove(int xPosition, int yPosition);
	void OnScroll(double xOffset, double yOffset);
	void OnLeftClickPressed();
	void OnLeftClickRelease();

	void OnCharPressed(unsigned int codePoint);
private:
};
