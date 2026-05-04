#pragma once

#include "Goknar/Core.h"
#include "Goknar/ObjectBase.h"

class InstancedStaticMeshComponent;

class GOKNAR_API TerrainGrassSpawner : public ObjectBase
{
public:
	TerrainGrassSpawner();

protected:
	virtual void BeginGame() override;

private:
	InstancedStaticMeshComponent* grassInstancedStaticMeshComponent_{ nullptr };
	InstancedStaticMeshComponent* treeInstancedStaticMeshComponent_{ nullptr };
};
