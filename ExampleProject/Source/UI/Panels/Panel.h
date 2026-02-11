#pragma once

#include <string>

#include "imgui.h"

#include "Goknar/Math/GoknarMath.h"

class HUD;

class IPanel
{
public:
	IPanel() = delete;
	IPanel(const std::string& title, HUD* hud) : 
		title_(title), 
		hud_(hud), 
		isOpen_(true)
	{

	}
	virtual ~IPanel() = default;

	virtual void Init() {}
	virtual void Draw() = 0;

	void SetIsOpen(bool isOpen)
	{
		isOpen_ = isOpen;
	}

	bool GetIsOpen() const
	{
		return isOpen_;
	}

	const std::string& GetTitle() const { return title_; }

	inline bool IsCursorOn()
	{
		if (!isOpen_)
		{
			return false;
		}

		ImVec2 mousePos = ImGui::GetMousePos();

		bool isMouseInside = mousePos.x >= position_.x && mousePos.x <= position_.x + size_.x &&
			mousePos.y >= position_.y && mousePos.y <= position_.y + size_.y;

		return isMouseInside;
	}

	const Vector2i& GetPosition() const
	{
		return position_;
	}

	const Vector2i& GetSize() const
	{
		return size_;
	}

protected:

	std::string title_{ "" };

	Vector2i position_{ Vector2i::ZeroVector };
	Vector2i size_{ Vector2i::ZeroVector };

	HUD* hud_{ nullptr };

	bool isOpen_{ true };
};