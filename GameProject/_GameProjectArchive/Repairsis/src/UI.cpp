#include "UI.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Model/Mesh.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Scene.h"

UI::UI()
{
	gameOverSprite_ = new StaticMeshComponent(this);
	gameOverSprite_->SetMesh(engine->GetApplication()->GetMainScene()->GetMesh(4));
	gameOverSprite_->SetRelativePosition(Vector3(-2.5f, 12.5f, 0.f));
	gameOverSprite_->SetRelativeScaling(Vector3(5.f, 5.f, 1.f));
	gameOverSprite_->SetIsRendered(false);

	restartSprite_ = new StaticMeshComponent(this);
	restartSprite_->SetMesh(engine->GetApplication()->GetMainScene()->GetMesh(5));
	restartSprite_->SetRelativePosition(Vector3(-2.5f, 7.5f, 0.f));
	restartSprite_->SetRelativeScaling(Vector3(5.f, 5.f, 1.f));
	restartSprite_->SetIsRendered(false);
}

UI::~UI()
{
	delete gameOverSprite_;
	delete restartSprite_;
}

void UI::ShowGameOverUI()
{
	gameOverSprite_->SetIsRendered(true);
	restartSprite_->SetIsRendered(true);
}

void UI::HideGameOverUI()
{
	gameOverSprite_->SetIsRendered(false);
	restartSprite_->SetIsRendered(false);
}
