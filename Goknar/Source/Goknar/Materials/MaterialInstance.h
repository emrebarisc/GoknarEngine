#ifndef __MATERIALINSTANCE_H__
#define __MATERIALINSTANCE_H__

#include "MaterialBase.h"

class Material;

class GOKNAR_API MaterialInstance : public IMaterialBase
{
public:
	~MaterialInstance();

	static MaterialInstance* Create(Material* parent);

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	virtual Shader* GetShader(RenderPassType renderPassType) const override;

protected:
	MaterialInstance();
	MaterialInstance(Material* parent);
	MaterialInstance(const MaterialInstance&) = delete;

	const MaterialInstance& operator =(const MaterialInstance&) = delete;

private:
	Material* parentMaterial_{ nullptr };
};

#endif