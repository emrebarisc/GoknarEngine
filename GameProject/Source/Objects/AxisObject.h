#pragma once

#include "Goknar/Core.h"
#include "Goknar/ObjectBase.h"

class StaticMeshComponent;

class GOKNAR_API AxisObject : public ObjectBase
{
public:
	AxisObject();

protected:
	virtual void BeginGame() override;

	StaticMeshComponent* staticMeshComponent;
private:
};