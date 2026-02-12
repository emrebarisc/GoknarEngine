#pragma once

#include "Panel.h"
#include "Goknar/Math/GoknarMath.h"

class BlendSpacePanel : public IPanel
{
public:
	BlendSpacePanel(HUD* hud);
	virtual ~BlendSpacePanel();

	virtual void Init() override;
	virtual void Draw() override;

private:
	float sliderValue{ 0.f };
};