#pragma once

#include <vector>

#include "Goknar/Core.h"
#include "Goknar/ObjectBase.h"

class MaterialSphere;

class GOKNAR_API MaterialSphereSpawner : public ObjectBase
{
public:
    MaterialSphereSpawner();
    ~MaterialSphereSpawner();

    virtual void BeginGame() override;

protected:

private:
    std::vector<MaterialSphere*> materialSpheres_;
};