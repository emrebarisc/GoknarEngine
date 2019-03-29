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

	void OnCursorMove(int xPosition, int yPosition);

	void OnLeftClickPressed();
	void OnLeftClickRelease();

private:
};
