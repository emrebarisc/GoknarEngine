#ifndef __STATICMESHLOD_H__
#define __STATICMESHLOD_H__

#include "MeshSection.h"
#include "MeshGeometry.h"

class GOKNAR_API StaticMeshLOD : public MeshSection<MeshGeometry>
{
public:
	StaticMeshLOD();

	virtual ~StaticMeshLOD();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

private:
};

#endif
