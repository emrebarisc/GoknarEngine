#ifndef __DYNAMICMESHLOD_H_
#define __DYNAMICMESHLOD_H_

#include "MeshSection.h"
#include "DynamicMeshGeometry.h"

class GOKNAR_API DynamicMeshLOD : public MeshSection<DynamicMeshGeometry>
{
public:
	DynamicMeshLOD();
	DynamicMeshLOD(const MeshSection<DynamicMeshGeometry>& mesh);
	virtual ~DynamicMeshLOD();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;
private:
};

#endif
