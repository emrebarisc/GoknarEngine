#include "BulletHitVFXObject.h"

#include "Goknar/Components/ParticleSystemComponent.h"

BulletHitVFXObject::BulletHitVFXObject() :
	ObjectBase()
{
	billboardParticleSystemComponent_ = AddSubComponent<BillboardParticleSystemComponent>();
}

void BulletHitVFXObject::BeginGame()
{
	ObjectBase::BeginGame();

	GPUParticleSpawnDesc gpuParticleSpawnDesc;

	gpuParticleSpawnDesc.looping = false;
	gpuParticleSpawnDesc.initialVelocity = GPUParticleValueRange(Vector3{ 1.f, -1.f, 0.f }, Vector3{ 2.f, 1.f, 2.f });
	gpuParticleSpawnDesc.emissiveColorByLifetime = GPUParticleVector3Curve{ Vector3{4.f, 2.f, 0.f}, Vector3::ZeroVector };
	gpuParticleSpawnDesc.spawnBoxExtents = Vector3{ 0.01f };
	gpuParticleSpawnDesc.lifetime = GPUParticleValueRange(0.5f, 1.f);

	billboardParticleSystemComponent_->GetParticleSystem()->SetSpawnDesc(gpuParticleSpawnDesc);
	billboardParticleSystemComponent_->GetParticleSystem()->QueueBurstSpawn(20);
	billboardParticleSystemComponent_->GetParticleSystem()->SetParticleSize(0.025f);
}