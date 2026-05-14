#pragma once

#include "Goknar/ObjectBase.h"

class BillboardParticleSystemComponent;

class GOKNAR_API BulletHitVFXObject : public ObjectBase
{
public:
	BulletHitVFXObject();
	void BeginGame() override;

protected:

private:
	BillboardParticleSystemComponent* billboardParticleSystemComponent_;
};