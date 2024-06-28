#include "MaterialTestObject.h"

#include "Goknar/Engine.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Model/StaticMesh.h"


MaterialTestObject::MaterialTestObject()
{
	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();
	StaticMesh* staticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Cube.fbx");
	if(staticMesh)
	{
		staticMeshComponent_->SetMesh(staticMesh);
	}

	StaticMeshInstance* staticMeshInstance = staticMeshComponent_->GetMeshInstance();
	IMaterialBase* materialBase = staticMeshInstance->GetMaterial();
	Material* material = dynamic_cast<Material*>(materialBase);

	MaterialInitializationData* materialInitializationData = material->GetInitializationData();

	materialInitializationData->vertexPositionOffset.calculation = 
R"(
	float time = )" + std::string(SHADER_VARIABLE_NAMES::TIMING::ELAPSED_TIME) + R"(;
	const float speed = 2.f;
	const float distance = 2.f;
	float sinValue = sin(time * speed);
	vec3 offset = vec3(0.f, 0.f, sinValue * distance);
)";
	materialInitializationData->vertexPositionOffset.result = "offset";

	//SetWorldScaling(Vector3{ 0.25f });
}

MaterialTestObject::~MaterialTestObject()
{

}

void MaterialTestObject::BeginGame()
{
	SetWorldPosition(Vector3{ 0.f, 0.f, 3.f });
}

