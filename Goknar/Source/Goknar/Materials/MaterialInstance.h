#ifndef __MATERIALINSTANCE_H__
#define __MATERIALINSTANCE_H__

#include "MaterialBase.h"

class Material;

class GOKNAR_API MaterialInstance : public IMaterialBase
{
	friend Material;
public:
	static MaterialInstance* Create(Material* parent);

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	virtual Shader* GetShader(RenderPassType renderPassType) const override;

	virtual void Destroy();

protected:
	MaterialInstance();
	MaterialInstance(Material* parent);
	MaterialInstance(const MaterialInstance&) = delete;
	virtual ~MaterialInstance();

	const MaterialInstance& operator =(const MaterialInstance&) = delete;

private:
	Material* parentMaterial_{ nullptr };
};

#endif