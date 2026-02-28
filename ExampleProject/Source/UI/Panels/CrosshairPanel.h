#pragma once

#include "Panel.h"

#include "Goknar/Delegates/Delegate.h"
#include "Goknar/Math/GoknarMath.h"

class CrosshairPanel : public IPanel
{
public:
	CrosshairPanel(HUD* hud);
	virtual ~CrosshairPanel();

	virtual void Init() override;
	virtual void Draw() override;

protected:
	void SetOpen()
	{
		SetIsOpen(true);
	};

	void SetClosed()
	{
		SetIsOpen(true);
	};

private:
	Delegate<void()> onGamePausedDelegate_;
	Delegate<void()> onGameResumedDelegate_;

	float sliderValue{ 0.f };
};