#ifndef __EDITORGROUNDGRID_H__
#define __EDITORGROUNDGRID_H__

#include "Goknar/ObjectBase.h"

class Material;
class StaticMesh;
class StaticMeshComponent;

class EditorGroundGrid : public ObjectBase
{
public:
	EditorGroundGrid();
	~EditorGroundGrid();

	void BeginGame() override;

	void Init();

protected:

private:
	Material* groundGridMeshMaterial_;
	StaticMesh* groundGridMesh_;
	StaticMeshComponent* gridMeshComponent_;
};

#endif