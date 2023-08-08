#ifndef __UI_H__
#define __UI_H__

#include "Goknar/ObjectBase.h"

class StaticMeshComponent;

class UI : public ObjectBase
{
public:
	UI();
	~UI();

	void ShowGameOverUI();
	void HideGameOverUI();

private:
	StaticMeshComponent* gameOverSprite_;
	StaticMeshComponent* restartSprite_;
};

#endif