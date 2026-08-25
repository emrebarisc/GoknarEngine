#ifndef __STATICMESHCOMPONENT_H__
#define __STATICMESHCOMPONENT_H__

#include "MeshComponent.h"

class StaticMesh;
class StaticMeshInstance;

using StaticMeshContainer = MeshContainer<StaticMesh>;

class GOKNAR_API StaticMeshComponent : public MeshComponent<StaticMeshContainer, StaticMeshInstance>
{
public:
	StaticMeshComponent() = delete;
	StaticMeshComponent(const StaticMeshComponent&) = delete;
	StaticMeshComponent(Component* parent);
	virtual ~StaticMeshComponent();

	Component* Clone() const override;
	void SetMesh(StaticMeshContainer* mesh) override;

protected:
private:
};
#endif
