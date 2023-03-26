#pragma once

#include "Goknar/Core.h"
#include "Goknar/ObjectBase.h"

class StaticMeshComponent;

class GOKNAR_API StaticMeshObject : public ObjectBase
{
public:
	StaticMeshObject();

protected:
	StaticMeshComponent* staticMeshComponent;
private:
};