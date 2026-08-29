#ifndef __PARTICLESYSTEMCOMPONENT_H__
#define __PARTICLESYSTEMCOMPONENT_H__

#include "Component.h"

#include "Goknar/ParticleSystem/ParticleSystemBase.h"

#include <cstdint>
#include <string>

class BillboardParticleSystem;
class Image;
class IMaterialBase;
class Material;
class ParticleSystemBase;
class StaticMesh;
class StaticMeshLOD;
class StaticMeshParticleSystem;

class GOKNAR_API ParticleSystemComponent : public Component
{
public:
	ParticleSystemComponent() = delete;
	ParticleSystemComponent(const ParticleSystemComponent&) = delete;
	explicit ParticleSystemComponent(Component* parent);
	~ParticleSystemComponent() override;

	void PreInit() override;
	void Init() override;
	void PostInit() override;
	void TickComponent(float deltaTime) override;
	void SetIsActive(bool isActive) override;
	Component* Clone() const override = 0;

	void SetMaxParticleCount(std::uint32_t maxParticleCount);
	std::uint32_t GetMaxParticleCount() const
	{
		return maxParticleCount_;
	}

	void SetGravity(const Vector3& gravity);
	const Vector3& GetGravity() const
	{
		return gravity_;
	}

	void SetParticleSize(float particleSize);
	float GetParticleSize() const
	{
		return particleSize_;
	}

	void SetSpawnDesc(const GPUParticleSpawnDesc& spawnDesc);
	const GPUParticleSpawnDesc& GetSpawnDesc() const
	{
		return spawnDesc_;
	}

	void SetPreviewParticleCount(std::uint32_t previewParticleCount);
	std::uint32_t GetPreviewParticleCount() const
	{
		return previewParticleCount_;
	}

	ParticleSystemBase* GetParticleSystem() const
	{
		return particleSystem_;
	}

	void RegeneratePreviewParticles();
	void ClearParticles();

protected:
	inline static constexpr float MINIMUN_PARTICLE_SIZE = 0.001f;
	inline static constexpr float MINIMUM_LIFETIME = 0.0001f;
	inline static constexpr float MINIMUM_SPAWN_INTERVAL = 0.0001f;

	void DestroyInner() override;
	void CopyParticleSystemValuesTo(ParticleSystemComponent* particleSystemComponent) const;

	virtual ParticleSystemBase* CreateParticleSystemInstance(const GPUParticleSystemDesc& particleSystemDesc) const = 0;
	virtual void SyncParticleSystemRenderSettings() const {}

	GPUParticleValueRange<float> SanitizeFloatRange(const GPUParticleValueRange<float>& range, float minimumValue);
	GPUParticleValueRange<Vector3> SanitizeVector3Range(const GPUParticleValueRange<Vector3>& range);
	Vector2 SanitizeRangeVector(const Vector2& range);
	GPUParticleFloatCurve SanitizeFloatCurve(const GPUParticleFloatCurve& curve, float minimumValue);
	Vector4 SanitizeParticleColor(const Vector4& color);
	GPUParticleColorCurve SanitizeColorCurve(const GPUParticleColorCurve& curve);
	Vector3 SanitizeEmissiveColor(const Vector3& color);
	GPUParticleVector3Curve SanitizeEmissiveColorCurve(const GPUParticleVector3Curve& curve);
	GPUParticleSpawnDesc SanitizeSpawnDesc(const GPUParticleSpawnDesc& spawnDesc);

private:

	void CreateParticleSystem();
	void DestroyParticleSystem();
	void RecreateParticleSystem();
	void SyncParticleSystemSettings() const;
	void SyncParticleSystemTransform() const;

	ParticleSystemBase* particleSystem_{ nullptr };

	std::uint32_t maxParticleCount_{ 65536u };
	std::uint32_t previewParticleCount_{ 0u };
	Vector3 gravity_{ 0.f, 0.f, -9.81f };
	float particleSize_{ 0.1f };
	GPUParticleSpawnDesc spawnDesc_{};
};

class GOKNAR_API BillboardParticleSystemComponent : public ParticleSystemComponent
{
public:
	BillboardParticleSystemComponent() = delete;
	explicit BillboardParticleSystemComponent(Component* parent);
	Component* Clone() const override;

	void SetBillboardTexture(Image* billboardTexture);
	const Image* GetBillboardTexture() const
	{
		return billboardTexture_;
	}

	void SetBillboardTexturePath(const std::string& billboardTexturePath);
	const std::string& GetBillboardTexturePath() const
	{
		return billboardTexturePath_;
	}

	void SetBillboardMaterialPath(const std::string& billboardMaterialPath);
	const std::string& GetBillboardMaterialPath() const
	{
		return billboardMaterialPath_;
	}

	const Material* GetBillboardMaterial() const
	{
		return billboardMaterial_;
	}

protected:
	ParticleSystemBase* CreateParticleSystemInstance(const GPUParticleSystemDesc& particleSystemDesc) const override;
	void SyncParticleSystemRenderSettings() const override;

private:
	void LoadBillboardTextureFromPath();
	void LoadBillboardMaterialFromPath();
	BillboardParticleSystem* GetBillboardParticleSystem() const;

	Image* billboardTexture_{ nullptr };
	Material* billboardMaterial_{ nullptr };
	std::string billboardTexturePath_{ "" };
	std::string billboardMaterialPath_{ "" };
};

class GOKNAR_API StaticMeshParticleSystemComponent : public ParticleSystemComponent
{
public:
	StaticMeshParticleSystemComponent() = delete;
	explicit StaticMeshParticleSystemComponent(Component* parent);
	Component* Clone() const override;

	void SetStaticMesh(StaticMeshLOD* staticMesh);
	const StaticMeshLOD* GetStaticMesh() const
	{
		return staticMesh_;
	}

	void SetStaticMeshPath(const std::string& staticMeshPath);
	const std::string& GetStaticMeshPath() const
	{
		return staticMeshPath_;
	}

protected:
	ParticleSystemBase* CreateParticleSystemInstance(const GPUParticleSystemDesc& particleSystemDesc) const override;
	void SyncParticleSystemRenderSettings() const override;

private:
	void LoadStaticMeshFromPath();
	StaticMeshParticleSystem* GetStaticMeshParticleSystem() const;

	StaticMeshLOD* staticMesh_{ nullptr };
	std::string staticMeshPath_{ "" };
};

#endif
