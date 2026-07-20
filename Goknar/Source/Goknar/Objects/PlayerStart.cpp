#include "pch.h"

#include "Goknar/Objects/PlayerStart.h"

PlayerStart::PlayerStart(const ObjectInitializer& objectInitializer) :
	ObjectBase(objectInitializer)
{
	SetName("PlayerStart");
}

PlayerStart::~PlayerStart()
{
}

ObjectBase* PlayerStart::Clone() const
{
	PlayerStart* clonedObject = new PlayerStart();
	CopyValuesTo(clonedObject);

	return clonedObject;
}

void PlayerStart::PreInit()
{
	ObjectBase::PreInit();
}

void PlayerStart::Init()
{
	ObjectBase::Init();
}

void PlayerStart::PostInit()
{
	ObjectBase::PostInit();
}
