 #include "pch.h"

#include "Material.h"
#include "MaterialInstance.h"

MaterialInstance::MaterialInstance()
{
}

MaterialInstance::MaterialInstance(Material* parent) :
	IMaterialBase(dynamic_cast<const IMaterialBase*>(parent))
{
	parentMaterial_ = parent;
	parent->AddDerivedMaterialInstance(this);
}

MaterialInstance::~MaterialInstance()
{
	parentMaterial_->RemoveDerivedMaterialInstance(this);
}

MaterialInstance* MaterialInstance::Create(Material* parent)
{
	return new MaterialInstance(parent);
}

void MaterialInstance::PreInit()
{

}

void MaterialInstance::Init()
{

}

void MaterialInstance::PostInit()
{

}

Shader* MaterialInstance::GetShader(RenderPassType renderPassType) const
{
	return parentMaterial_->GetShader(renderPassType);
}
