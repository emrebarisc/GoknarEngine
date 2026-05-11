#include "pch.h"

#include "ParticleSystemComponent.h"

#include <algorithm>

#include "Goknar/Contents/Image.h"
#include "Goknar/Engine.h"
#include "Goknar/Helpers/ContentPathUtils.h"
#include "Goknar/Helpers/SceneParser.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/ParticleSystem/BillboardParticleSystem.h"
#include "Goknar/ParticleSystem/StaticMeshParticleSystem.h"

namespace
{
	constexpr float kMinimumParticleSize = 0.001f;
	constexpr float kMinimumLifetime = 0.0001f;
	constexpr float kMinimumSpawnInterval = 0.0001f;

	GPUParticleValueRange<float> SanitizeFloatRange(const GPUParticleValueRange<float>& range, float minimumValue)
	{
		GPUParticleValueRange<float> sanitizedRange(range);
		sanitizedRange.minValue = (std::max)(minimumValue, sanitizedRange.minValue);
		sanitizedRange.maxValue = (std::max)(minimumValue, sanitizedRange.maxValue);
		if (sanitizedRange.maxValue < sanitizedRange.minValue)
		{
			std::swap(sanitizedRange.minValue, sanitizedRange.maxValue);
		}

		return sanitizedRange;
	}

	GPUParticleValueRange<Vector3> SanitizeVector3Range(const GPUParticleValueRange<Vector3>& range)
	{
		return GPUParticleValueRange<Vector3>(
			Vector3::Min(range.minValue, range.maxValue),
			Vector3::Max(range.minValue, range.maxValue));
	}

	Vector2 SanitizeRangeVector(const Vector2& range)
	{
		return Vector2((std::min)(range.x, range.y), (std::max)(range.x, range.y));
	}

	GPUParticleFloatCurve SanitizeFloatCurve(const GPUParticleFloatCurve& curve, float minimumValue)
	{
		GPUParticleFloatCurve sanitizedCurve(curve);
		sanitizedCurve.startValue = (std::max)(minimumValue, sanitizedCurve.startValue);
		sanitizedCurve.endValue = (std::max)(minimumValue, sanitizedCurve.endValue);
		return sanitizedCurve;
	}

	Vector4 SanitizeParticleColor(const Vector4& color)
	{
		Vector4 sanitizedColor(color);
		sanitizedColor.w = GoknarMath::Clamp(sanitizedColor.w, 0.f, 1.f);
		return sanitizedColor;
	}

	GPUParticleColorCurve SanitizeColorCurve(const GPUParticleColorCurve& curve)
	{
		GPUParticleColorCurve sanitizedCurve(curve);
		sanitizedCurve.startValue = SanitizeParticleColor(sanitizedCurve.startValue);
		sanitizedCurve.endValue = SanitizeParticleColor(sanitizedCurve.endValue);
		return sanitizedCurve;
	}

	GPUParticleSpawnDesc SanitizeSpawnDesc(const GPUParticleSpawnDesc& spawnDesc)
	{
		GPUParticleSpawnDesc sanitizedSpawnDesc(spawnDesc);
		sanitizedSpawnDesc.lifetime = SanitizeFloatRange(sanitizedSpawnDesc.lifetime, kMinimumLifetime);
		sanitizedSpawnDesc.initialVelocity = SanitizeVector3Range(sanitizedSpawnDesc.initialVelocity);
		sanitizedSpawnDesc.initialRotation = SanitizeVector3Range(sanitizedSpawnDesc.initialRotation);
		sanitizedSpawnDesc.angularVelocity = SanitizeVector3Range(sanitizedSpawnDesc.angularVelocity);
		sanitizedSpawnDesc.acceleration = SanitizeVector3Range(sanitizedSpawnDesc.acceleration);
		sanitizedSpawnDesc.velocityLimit = (std::max)(0.f, sanitizedSpawnDesc.velocityLimit);
		sanitizedSpawnDesc.sizeByLifetime = SanitizeFloatCurve(sanitizedSpawnDesc.sizeByLifetime, 0.f);
		sanitizedSpawnDesc.sizeBySpeedRange = SanitizeRangeVector(sanitizedSpawnDesc.sizeBySpeedRange);
		sanitizedSpawnDesc.sizeBySpeed = SanitizeFloatCurve(sanitizedSpawnDesc.sizeBySpeed, 0.f);
		sanitizedSpawnDesc.colorByLifetime = SanitizeColorCurve(sanitizedSpawnDesc.colorByLifetime);
		sanitizedSpawnDesc.colorBySpeedRange = SanitizeRangeVector(sanitizedSpawnDesc.colorBySpeedRange);
		sanitizedSpawnDesc.colorBySpeed = SanitizeColorCurve(sanitizedSpawnDesc.colorBySpeed);
		sanitizedSpawnDesc.spawnBoxExtents.x = (std::max)(0.f, sanitizedSpawnDesc.spawnBoxExtents.x);
		sanitizedSpawnDesc.spawnBoxExtents.y = (std::max)(0.f, sanitizedSpawnDesc.spawnBoxExtents.y);
		sanitizedSpawnDesc.spawnBoxExtents.z = (std::max)(0.f, sanitizedSpawnDesc.spawnBoxExtents.z);
		sanitizedSpawnDesc.spawnInterval = (std::max)(kMinimumSpawnInterval, sanitizedSpawnDesc.spawnInterval);
		sanitizedSpawnDesc.spawnCountPerInterval = (std::max)(1u, sanitizedSpawnDesc.spawnCountPerInterval);
		return sanitizedSpawnDesc;
	}
}

ParticleSystemComponent::ParticleSystemComponent(Component* parent) :
	Component(parent)
{
	SetIsTickable(true);
}

ParticleSystemComponent::~ParticleSystemComponent()
{
	DestroyParticleSystem();
}

void ParticleSystemComponent::PreInit()
{
	Component::PreInit();

	CreateParticleSystem();
	if (particleSystem_)
	{
		particleSystem_->PreInit();
	}
}

void ParticleSystemComponent::Init()
{
	Component::Init();

	SyncParticleSystemSettings();
	if (particleSystem_)
	{
		particleSystem_->Init();
	}
}

void ParticleSystemComponent::PostInit()
{
	Component::PostInit();

	if (particleSystem_)
	{
		particleSystem_->PostInit();
	}

	SyncParticleSystemSettings();
	RegeneratePreviewParticles();
}

void ParticleSystemComponent::TickComponent(float)
{
	SyncParticleSystemTransform();
}

void ParticleSystemComponent::SetIsActive(bool isActive)
{
	if (GetIsActive() == isActive)
	{
		return;
	}

	Component::SetIsActive(isActive);

	if (particleSystem_)
	{
		particleSystem_->SetIsActive(isActive);
	}
}

void ParticleSystemComponent::SetMaxParticleCount(std::uint32_t maxParticleCount)
{
	const std::uint32_t sanitizedMaxParticleCount = (std::max)(1u, maxParticleCount);
	if (maxParticleCount_ == sanitizedMaxParticleCount)
	{
		return;
	}

	maxParticleCount_ = sanitizedMaxParticleCount;
	previewParticleCount_ = (std::min)(previewParticleCount_, maxParticleCount_);

	if (GetIsInitialized())
	{
		RecreateParticleSystem();
	}
}

void ParticleSystemComponent::SetGravity(const Vector3& gravity)
{
	if (gravity_.Equals(gravity))
	{
		return;
	}

	gravity_ = gravity;

	if (particleSystem_)
	{
		particleSystem_->SetGravity(gravity_);
	}
}

void ParticleSystemComponent::SetParticleSize(float particleSize)
{
	const float sanitizedParticleSize = (std::max)(kMinimumParticleSize, particleSize);
	if (particleSize_ == sanitizedParticleSize)
	{
		return;
	}

	particleSize_ = sanitizedParticleSize;

	if (particleSystem_)
	{
		particleSystem_->SetParticleSize(particleSize_);
	}
}

void ParticleSystemComponent::SetSpawnDesc(const GPUParticleSpawnDesc& spawnDesc)
{
	spawnDesc_ = SanitizeSpawnDesc(spawnDesc);

	if (particleSystem_)
	{
		particleSystem_->SetSpawnDesc(spawnDesc_);
	}
}

void ParticleSystemComponent::SetPreviewParticleCount(std::uint32_t previewParticleCount)
{
	const std::uint32_t sanitizedPreviewParticleCount = (std::min)(previewParticleCount, maxParticleCount_);
	if (previewParticleCount_ == sanitizedPreviewParticleCount)
	{
		return;
	}

	previewParticleCount_ = sanitizedPreviewParticleCount;
}

void ParticleSystemComponent::RegeneratePreviewParticles()
{
	if (!particleSystem_ || !particleSystem_->GetIsInitialized())
	{
		return;
	}

	SyncParticleSystemSettings();
	particleSystem_->ClearAllParticles(false);
	particleSystem_->QueueBurstSpawn(previewParticleCount_);
	particleSystem_->Tick(0.f);
}

void ParticleSystemComponent::ClearParticles()
{
	if (!particleSystem_ || !particleSystem_->GetIsInitialized())
	{
		return;
	}

	particleSystem_->ClearAllParticles();
}

void ParticleSystemComponent::DestroyInner()
{
	DestroyParticleSystem();
	Component::DestroyInner();
}

void ParticleSystemComponent::CopyParticleSystemValuesTo(ParticleSystemComponent* particleSystemComponent) const
{
	if (!particleSystemComponent)
	{
		return;
	}

	CopyValuesTo(particleSystemComponent);
	particleSystemComponent->SetMaxParticleCount(maxParticleCount_);
	particleSystemComponent->SetGravity(gravity_);
	particleSystemComponent->SetParticleSize(particleSize_);
	particleSystemComponent->SetSpawnDesc(spawnDesc_);
	particleSystemComponent->SetPreviewParticleCount(previewParticleCount_);
}

void ParticleSystemComponent::CreateParticleSystem()
{
	if (particleSystem_)
	{
		return;
	}

	GPUParticleSystemDesc particleSystemDesc;
	particleSystemDesc.maxParticleCount = maxParticleCount_;
	particleSystemDesc.gravity = gravity_;
	particleSystemDesc.particleSize = particleSize_;
	particleSystemDesc.spawnDesc = spawnDesc_;

	particleSystem_ = CreateParticleSystemInstance(particleSystemDesc);
	SyncParticleSystemSettings();
}

void ParticleSystemComponent::DestroyParticleSystem()
{
	delete particleSystem_;
	particleSystem_ = nullptr;
}

void ParticleSystemComponent::RecreateParticleSystem()
{
	DestroyParticleSystem();
	CreateParticleSystem();

	if (!particleSystem_)
	{
		return;
	}

	particleSystem_->PreInit();
	particleSystem_->Init();
	particleSystem_->PostInit();

	SyncParticleSystemSettings();
	RegeneratePreviewParticles();
}

void ParticleSystemComponent::SyncParticleSystemSettings() const
{
	if (!particleSystem_)
	{
		return;
	}

	particleSystem_->SetGravity(gravity_);
	particleSystem_->SetParticleSize(particleSize_);
	particleSystem_->SetSpawnDesc(spawnDesc_);
	particleSystem_->SetIsActive(GetIsActive());

	SyncParticleSystemRenderSettings();
	SyncParticleSystemTransform();
}

void ParticleSystemComponent::SyncParticleSystemTransform() const
{
	if (!particleSystem_)
	{
		return;
	}

	particleSystem_->SetRenderTransformMatrix(GetComponentToWorldTransformationMatrix());
}

BillboardParticleSystemComponent::BillboardParticleSystemComponent(Component* parent) :
	ParticleSystemComponent(parent)
{
}

Component* BillboardParticleSystemComponent::Clone() const
{
	BillboardParticleSystemComponent* clonedComponent = new BillboardParticleSystemComponent((Component*)nullptr);
	CopyParticleSystemValuesTo(clonedComponent);
	if (!billboardTexturePath_.empty())
	{
		clonedComponent->SetBillboardTexturePath(billboardTexturePath_);
	}
	else
	{
		clonedComponent->SetBillboardTexture(billboardTexture_);
	}
	clonedComponent->SetBillboardMaterialPath(billboardMaterialPath_);

	return clonedComponent;
}

void BillboardParticleSystemComponent::SetBillboardTexture(Image* billboardTexture)
{
	if (billboardTexture_ == billboardTexture)
	{
		return;
	}

	billboardTexture_ = billboardTexture;
	billboardTexturePath_ = billboardTexture_ ? ContentPathUtils::ToContentRelativePath(billboardTexture_->GetPath()) : "";

	if (BillboardParticleSystem* billboardParticleSystem = GetBillboardParticleSystem())
	{
		billboardParticleSystem->SetParticleTexture(billboardTexture_);
	}
}

void BillboardParticleSystemComponent::SetBillboardTexturePath(const std::string& billboardTexturePath)
{
	const std::string normalizedBillboardTexturePath = ContentPathUtils::ToContentRelativePath(billboardTexturePath);
	if (billboardTexturePath_ == normalizedBillboardTexturePath)
	{
		return;
	}

	billboardTexturePath_ = normalizedBillboardTexturePath;
	LoadBillboardTextureFromPath();

	if (BillboardParticleSystem* billboardParticleSystem = GetBillboardParticleSystem())
	{
		billboardParticleSystem->SetParticleTexture(billboardTexture_);
	}
}

void BillboardParticleSystemComponent::SetBillboardMaterialPath(const std::string& billboardMaterialPath)
{
	const std::string normalizedBillboardMaterialPath = ContentPathUtils::ToContentRelativePath(billboardMaterialPath);
	if (billboardMaterialPath_ == normalizedBillboardMaterialPath)
	{
		return;
	}

	billboardMaterialPath_ = normalizedBillboardMaterialPath;
	LoadBillboardMaterialFromPath();

	if (BillboardParticleSystem* billboardParticleSystem = GetBillboardParticleSystem())
	{
		billboardParticleSystem->SetParticleMaterial(billboardMaterial_);
	}
}

ParticleSystemBase* BillboardParticleSystemComponent::CreateParticleSystemInstance(const GPUParticleSystemDesc& particleSystemDesc) const
{
	return new BillboardParticleSystem(particleSystemDesc);
}

void BillboardParticleSystemComponent::SyncParticleSystemRenderSettings() const
{
	if (BillboardParticleSystem* billboardParticleSystem = GetBillboardParticleSystem())
	{
		billboardParticleSystem->SetParticleTexture(billboardTexture_);
		billboardParticleSystem->SetParticleMaterial(billboardMaterial_);
	}
}

void BillboardParticleSystemComponent::LoadBillboardTextureFromPath()
{
	billboardTexture_ = billboardTexturePath_.empty() ? nullptr : engine->GetResourceManager()->GetContent<Image>(billboardTexturePath_);
}

void BillboardParticleSystemComponent::LoadBillboardMaterialFromPath()
{
	billboardMaterial_ = billboardMaterialPath_.empty() ? nullptr : SceneParser::GetOrCreateSharedMaterial(billboardMaterialPath_);
}

BillboardParticleSystem* BillboardParticleSystemComponent::GetBillboardParticleSystem() const
{
	return dynamic_cast<BillboardParticleSystem*>(GetParticleSystem());
}

StaticMeshParticleSystemComponent::StaticMeshParticleSystemComponent(Component* parent) :
	ParticleSystemComponent(parent)
{
}

Component* StaticMeshParticleSystemComponent::Clone() const
{
	StaticMeshParticleSystemComponent* clonedComponent = new StaticMeshParticleSystemComponent((Component*)nullptr);
	CopyParticleSystemValuesTo(clonedComponent);
	if (!staticMeshPath_.empty())
	{
		clonedComponent->SetStaticMeshPath(staticMeshPath_);
	}
	else
	{
		clonedComponent->SetStaticMesh(staticMesh_);
	}

	return clonedComponent;
}

void StaticMeshParticleSystemComponent::SetStaticMesh(StaticMesh* staticMesh)
{
	if (staticMesh_ == staticMesh)
	{
		return;
	}

	staticMesh_ = staticMesh;
	staticMeshPath_ = staticMesh_ ? ContentPathUtils::ToContentRelativePath(staticMesh_->GetPath()) : "";

	if (StaticMeshParticleSystem* staticMeshParticleSystem = GetStaticMeshParticleSystem())
	{
		staticMeshParticleSystem->SetStaticMesh(staticMesh_);
	}
}

void StaticMeshParticleSystemComponent::SetStaticMeshPath(const std::string& staticMeshPath)
{
	const std::string normalizedStaticMeshPath = ContentPathUtils::ToContentRelativePath(staticMeshPath);
	if (staticMeshPath_ == normalizedStaticMeshPath)
	{
		return;
	}

	staticMeshPath_ = normalizedStaticMeshPath;
	LoadStaticMeshFromPath();

	if (StaticMeshParticleSystem* staticMeshParticleSystem = GetStaticMeshParticleSystem())
	{
		staticMeshParticleSystem->SetStaticMesh(staticMesh_);
	}
}

ParticleSystemBase* StaticMeshParticleSystemComponent::CreateParticleSystemInstance(const GPUParticleSystemDesc& particleSystemDesc) const
{
	return new StaticMeshParticleSystem(particleSystemDesc);
}

void StaticMeshParticleSystemComponent::SyncParticleSystemRenderSettings() const
{
	if (StaticMeshParticleSystem* staticMeshParticleSystem = GetStaticMeshParticleSystem())
	{
		staticMeshParticleSystem->SetStaticMesh(staticMesh_);
	}
}

void StaticMeshParticleSystemComponent::LoadStaticMeshFromPath()
{
	staticMesh_ = staticMeshPath_.empty() ? nullptr : engine->GetResourceManager()->GetContent<StaticMesh>(staticMeshPath_);
}

StaticMeshParticleSystem* StaticMeshParticleSystemComponent::GetStaticMeshParticleSystem() const
{
	return dynamic_cast<StaticMeshParticleSystem*>(GetParticleSystem());
}
