#include "time.h"
#include "RectanglePart.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Math.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Scene.h"

RectanglePart::RectanglePart()
{
	srand(time(0));
	int type = rand() % 4;

	type_ = type;
	//std::cout << type_ << std::endl;
	SetWorldPosition(Vector3(0.f, 16.f, 0.f));
	//SetWorldRotation(Vector3(0.f, 0.f, PI * (rand() % 4)));

	meshComponent_ = new StaticMeshComponent(this);

	meshComponent_->SetMesh(engine->GetApplication()->GetMainScene()->GetMesh(type_));
}


RectanglePart::~RectanglePart()
{
}

void RectanglePart::SetParent(RepairableObject* parent)
{
	parentObject_ = parent;
}

RepairableObject* RectanglePart::GetParent( )
{
	return parentObject_;
}
