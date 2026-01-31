#include "MaterialInitializer.h"

#include "Goknar/Engine.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Model/StaticMesh.h"

void MaterialInitializer::Init()
{
	DefaultSceneAssets();
}

void MaterialInitializer::DefaultSceneAssets()
{
    ResourceManager* resourceManager = engine->GetResourceManager();

    std::string placeholderTextureName = "placeholderTexture";

    Image* placeholderImage = resourceManager->GetContent<Image>("Textures/T_LightPlaceholder.png");
    placeholderImage->SetName(placeholderTextureName);

    Material* material = new Material();
    material->SetBaseColor(Vector3{ 0.f });
    material->SetEmmisiveColor(Vector3{ 0.f });
    material->SetSpecularReflectance(Vector3{ 1.f });
    material->AddTextureImage(placeholderImage);

    MaterialInitializationData* materialInitializationData = material->GetInitializationData();

    materialInitializationData->baseColor.calculation = R"(
    mat4 modelMatrix = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + R"(;

    float scaleX = length(vec3(modelMatrix[0].x, modelMatrix[0].y, modelMatrix[0].z));
    float scaleY = length(vec3(modelMatrix[1].x, modelMatrix[1].y, modelMatrix[1].z));
    float scaleZ = length(vec3(modelMatrix[2].x, modelMatrix[2].y, modelMatrix[2].z));
    vec3 worldScale = vec3(scaleX, scaleY, scaleZ);

    mat4 invModel = inverse(modelMatrix);

    vec3 worldNormal = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + R"(;
    vec3 localNormal = normalize((vec4(worldNormal, 0.0) * modelMatrix).xyz); 
    vec3 localPos = (invModel * )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"().xyz;
    
    vec3 absNormal = abs(localNormal);
    
    vec2 projectedUV;
    if (absNormal.x > absNormal.y && absNormal.x > absNormal.z)
    {
        projectedUV = localPos.zy * worldScale.zy;
    }
    else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z)
    {
        projectedUV = localPos.xz * worldScale.xz;
    }
    else
    {
        projectedUV = localPos.xy * worldScale.xy;
    }

    vec4 textureValue = texture2D()" + placeholderTextureName + R"(, projectedUV);
    float upsideValue = clamp(dot(vec3(0.0, 0.0, 1.0), worldNormal), 0.0, 1.0);
    textureValue.xyz = textureValue.xyz * upsideValue + vec3(0.05) * (1.0 - upsideValue);
)";

    materialInitializationData->baseColor.result = "textureValue;";

    StaticMesh* archMesh = resourceManager->GetContent<StaticMesh>("Meshes/DefaultScene/SM_Arch.fbx");
    archMesh->SetMaterial(material);

    StaticMesh* radialStaircaseMesh = resourceManager->GetContent<StaticMesh>("Meshes/DefaultScene/SM_RadialStaircase.fbx");
    radialStaircaseMesh->SetMaterial(material);

    StaticMesh* radialStaircaseMirroredMesh = resourceManager->GetContent<StaticMesh>("Meshes/DefaultScene/SM_RadialStaircaseMirrored.fbx");
    radialStaircaseMirroredMesh->SetMaterial(material);

    StaticMesh* roundCornerMesh = resourceManager->GetContent<StaticMesh>("Meshes/DefaultScene/SM_RoundCorner.fbx");
    roundCornerMesh->SetMaterial(material);

    StaticMesh* unitCubeMesh = resourceManager->GetContent<StaticMesh>("Meshes/DefaultScene/SM_UnitCube.fbx");
    unitCubeMesh->SetMaterial(material);
}

void MaterialInitializer::DefaultCharacter()
{
    Material* material = new Material();
    material->SetBaseColor(Vector3{ 0.f });
    material->SetEmmisiveColor(Vector3{ 0.f });
    material->SetSpecularReflectance(Vector3{ 1.f });

    ResourceManager* resourceManager = engine->GetResourceManager();

    SkeletalMesh* defaultCharacterMesh = resourceManager->GetContent<SkeletalMesh>("Meshes/SK_DefaultCharacter.fbx");
    defaultCharacterMesh->SetMaterial(material);
}
