#pragma once

#include "Panel.h"

class CrosshairPanel : public IPanel
{
public:
	CrosshairPanel(HUD* hud);
	virtual ~CrosshairPanel();

	virtual void Init() override;
	virtual void Draw() override;

protected:
private:
};