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
    DefaultCharacter();
    Matilda();
}

void MaterialInitializer::DefaultSceneAssets()
{
    ResourceManager* resourceManager = engine->GetResourceManager();

    std::string placeholderTextureName = "placeholderTexture";

    Image* placeholderImage = resourceManager->GetContent<Image>("Textures/T_DarkPlaceholder.png");
    placeholderImage->SetName(placeholderTextureName);

    Material* material = new Material();
    material->SetBaseColor(Vector3{ 0.f });
    material->SetEmmisiveColor(Vector3{ 0.f });
    material->SetSpecularReflectance(Vector3{ 1.f });
    material->AddTextureImage(placeholderImage);

    MaterialInitializationData* materialInitializationData = material->GetInitializationData();

    materialInitializationData->baseColor.calculation = R"(
    mat4 modelMatrix = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FINAL_MODEL_MATRIX) + R"(;

    mat4 invModel = inverse(modelMatrix);

    vec3 worldNormal = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::VERTEX_NORMAL) + R"(;
    vec3 fragmentPosition = )" + std::string(SHADER_VARIABLE_NAMES::VERTEX_SHADER_OUTS::FRAGMENT_POSITION_WORLD_SPACE) + R"(.xyz;
    
    vec3 absNormal = abs(worldNormal);
    
    vec2 projectedUV;
    if (absNormal.x > absNormal.y && absNormal.x > absNormal.z)
    {
        projectedUV = fragmentPosition.zy;
    }
    else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z)
    {
        projectedUV = fragmentPosition.xz;
    }
    else
    {
        projectedUV = fragmentPosition.xy;
    }

    vec4 textureValue = texture2D()" + placeholderTextureName + R"(, projectedUV);
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

    Material* blockMaterial = new Material();
    blockMaterial->SetBaseColor(Vector3{ 0.8f, 0.35f, 0.05f });
    blockMaterial->SetEmmisiveColor(Vector3{ 0.f });
    blockMaterial->SetSpecularReflectance(Vector3{ 2.f });
    blockMaterial->SetPhongExponent(2.f);
    StaticMesh* blockMesh = resourceManager->GetContent<StaticMesh>("Meshes/DefaultScene/SM_Block.fbx");
    blockMesh->SetMaterial(blockMaterial);
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

void MaterialInitializer::Matilda()
{
    ResourceManager* resourceManager = engine->GetResourceManager();

    std::string matildaTextureName = "matildaTexture";

    Image* matildaTexture = resourceManager->GetContent<Image>("Textures/T_Matilda.png");
    matildaTexture->SetName(matildaTextureName);

    Material* matildaMaterial = new Material();
    matildaMaterial->SetBaseColor(Vector3::ZeroVector);
    matildaMaterial->SetEmmisiveColor(Vector3{ 0.f });
    matildaMaterial->SetSpecularReflectance(Vector3{ 0.f });
    matildaMaterial->AddTextureImage(matildaTexture);

    StaticMesh* matildaMesh = resourceManager->GetContent<StaticMesh>("Meshes/SM_Matilda.fbx");
    matildaMesh->SetMaterial(matildaMaterial);
}
