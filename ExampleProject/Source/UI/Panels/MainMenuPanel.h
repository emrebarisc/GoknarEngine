#pragma once

#include "Panel.h"
#include "Goknar/Math/GoknarMath.h"

class MainMenuPanel : public IPanel
{
public:
	MainMenuPanel(HUD* hud);
	virtual ~MainMenuPanel();

	virtual void Init() override;
	virtual void Draw() override;

private:
};