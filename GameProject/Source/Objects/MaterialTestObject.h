#pragma once

#include "Goknar/Core.h"
#include "Goknar/ObjectBase.h"

class StaticMeshComponent;

class GOKNAR_API MaterialTestObject : public ObjectBase
{
public:
	MaterialTestObject();
	~MaterialTestObject();

	virtual void BeginGame() override;

protected:
	StaticMeshComponent* staticMeshComponent_;
private:
};