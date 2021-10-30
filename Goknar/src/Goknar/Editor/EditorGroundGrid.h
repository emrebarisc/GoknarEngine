#ifndef __EDITORGROUNDGRID_H__
#define __EDITORGROUNDGRID_H__

#include "Goknar/ObjectBase.h"

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
	StaticMeshComponent* gridMeshComponent_;
};

#endif