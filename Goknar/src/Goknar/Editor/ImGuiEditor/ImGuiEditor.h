#pragma once

#include "../Editor.h"

class GOKNAR_API ImGuiEditor : public Editor
{
public:
	ImGuiEditor();
	~ImGuiEditor();

	void Init() override;
	void Tick(float deltaTime) override;
};