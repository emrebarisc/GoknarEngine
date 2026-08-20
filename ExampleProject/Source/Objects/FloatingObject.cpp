#include "FloatingObject.h"

#include "Goknar/Engine.h"
#include "Goknar/Components/ParticleSystemComponent.h"

FloatingObject::FloatingObject()
{
	SetIsTickable(true);
	SetIsTickEnabled(true);

	particleSystemComponent_ = AddSubComponent<BillboardParticleSystemComponent>();
	particleSystemComponent_->SetPreviewParticleCount(0);

	particleSystemComponent_->SetBillboardMaterialPath("Materials/VFX/M_Particle");
}

FloatingObject::~FloatingObject()
{
	
}

void FloatingObject::BeginGame()
{
	initialPosition_ = GetWorldPosition();
}

void FloatingObject::Tick(float)
{
	SetWorldPosition(initialPosition_ + Vector3{0.f, 2.f, 0.f} * GoknarMath::Cos(0.5f * engine->GetElapsedTime()));
}
