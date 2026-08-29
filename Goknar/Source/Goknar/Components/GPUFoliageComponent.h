#ifndef __GPUFOLIAGECOMPONENT_H__
#define __GPUFOLIAGECOMPONENT_H__

#include "Goknar/Components/Component.h"
#include "Goknar/Renderer/GPUFoliageSystem.h"

#include <vector>

class GOKNAR_API GPUFoliageComponent : public Component
{
public:
	GPUFoliageComponent() = delete;
	GPUFoliageComponent(const GPUFoliageComponent&) = delete;
	explicit GPUFoliageComponent(Component* parent);
	~GPUFoliageComponent() override;

	Component* Clone() const override;

	void PreInit() override;
	void PostInit() override;
	void SetIsActive(bool isActive) override;
	void DestroyInner() override;

	void SetStaticMesh(const StaticMesh* staticMesh);
	const StaticMesh* GetStaticMesh() const;

	void SetInstances(const std::vector<GPUFoliageInstance>& instances, bool recalculateBounds = true);
	void SetInstanceTransformations(const std::vector<Matrix>& instanceTransformations, bool recalculateBounds = true);
	void AddInstance(const GPUFoliageInstance& instance, bool recalculateBounds = true);
	void AddInstanceTransformation(const Matrix& instanceTransformation, bool recalculateBounds = true);
	void ClearInstances();

	const std::vector<GPUFoliageInstance>& GetInstances() const;
	std::size_t GetInstanceCount() const;

	void SetCastsShadow(bool castsShadow);
	bool GetCastsShadow() const;

	GPUFoliageSystem* GetFoliageSystem() const
	{
		return foliageSystem_;
	}

protected:
	void UpdateComponentToWorldTransformationMatrix() override;

private:
	void EnsureFoliageSystem();
	void DestroyFoliageSystem();
	void SyncTransformToSystem() const;

	GPUFoliageSystem* foliageSystem_{ nullptr };
};

#endif
