#pragma once

#include "Goknar/Math/GoknarMath.h"

#include "imgui.h"

#include "UI/UIContext.h"

namespace UIUtils
{
	static inline Vector2 ToVector2(const ImVec2& value)
	{
		return {value.x, value.y};
	}

	static inline ImVec2 ToImVec2(const Vector2& value)
	{
		return {value.x, value.y};
	}

	static inline ImVec2 ToImVec2(const Vector2i& value)
	{
		return {(float)value.x, (float)value.y};
	}

	static bool IsCursorInCurrentWindow()
	{
		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 windowSize = ImGui::GetWindowSize();

		ImVec2 mousePos = ImGui::GetMousePos();

		bool isMouseInside = mousePos.x >= windowPos.x && mousePos.x <= windowPos.x + windowSize.x &&
			mousePos.y >= windowPos.y && mousePos.y <= windowPos.y + windowSize.y;

		return isMouseInside;
	}
}