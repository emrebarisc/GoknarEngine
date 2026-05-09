#pragma once

#include "Goknar/Core.h"
#include "Goknar/ObjectBase.h"

class BillboardParticleSystemComponent;

class GOKNAR_API FloatingObject : public ObjectBase
{
public:
	FloatingObject();
	~FloatingObject();

	virtual void BeginGame() override;
	virtual void Tick(float) override;

protected:

private:
	Vector3 initialPosition_{ Vector3::ZeroVector };

	BillboardParticleSystemComponent* particleSystemComponent_{ nullptr };
};
