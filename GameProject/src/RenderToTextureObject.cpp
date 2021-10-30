#include "RenderToTextureObject.h"

#include <cmath>

#include "Goknar/Application.h"
#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Scene.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/ShaderBuilder.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Renderer/Texture.h"

RenderToTextureObject::RenderToTextureObject() : ObjectBase()
{
	renderToTextureMeshComponent = new StaticMeshComponent(this);

	Scene* scene = engine->GetApplication()->GetMainScene();

	Texture* sampleTexture = new Texture();
	sampleTexture->SetName("sampleTexture");
	sampleTexture->SetTextureImagePath("./Content/Textures/CENG_Floor4.JPG");
	scene->AddTexture(sampleTexture);

	Shader* renderToTextureMeshShader = new Shader();
	renderToTextureMeshShader->SetShaderType(ShaderType::SelfContained);
	renderToTextureMeshShader->SetVertexShaderPath("./Content/Shaders/TextureVertexShader.glsl");
	renderToTextureMeshShader->SetFragmentShaderPath("./Content/Shaders/TextureFragmentShader.glsl");

	renderToTextureMeshShader->AddTexture(scene->GetStaticDirectionalLights()[0]->GetShadowMapTexture());
	renderToTextureMeshShader->AddTexture(sampleTexture);
	scene->AddShader(renderToTextureMeshShader);

	Material* renderToTextureMeshMaterial = new Material();
	renderToTextureMeshMaterial->SetDiffuseReflectance(Vector3(1.f, 0.f, 0.f));
	renderToTextureMeshMaterial->SetAmbientReflectance(Vector3(0.5f, 0.5f, 0.5f));
	renderToTextureMeshMaterial->SetSpecularReflectance(Vector3(0.15f, 0.15f, 0.15f));
	renderToTextureMeshMaterial->SetPhongExponent(1.f);
	renderToTextureMeshMaterial->SetShadingModel(MaterialShadingModel::TwoSided);
	renderToTextureMeshMaterial->SetShader(renderToTextureMeshShader);
	scene->AddMaterial(renderToTextureMeshMaterial);

	StaticMesh* renderToTextureMesh = new StaticMesh();
	renderToTextureMesh->AddVertexData(VertexData(Vector3(-1.f, 1.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(0.f, 1.f)));
	renderToTextureMesh->AddVertexData(VertexData(Vector3(1.f, 1.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(1.f, 1.f)));
	renderToTextureMesh->AddVertexData(VertexData(Vector3(-1.f, -1.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(0.f, 0.f)));
	renderToTextureMesh->AddVertexData(VertexData(Vector3(1.f, -1.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(1.f, 0.f)));
	renderToTextureMesh->AddFace(Face(1, 0, 2));
	renderToTextureMesh->AddFace(Face(1, 2, 3));
	renderToTextureMesh->SetMaterial(renderToTextureMeshMaterial);
	scene->AddStaticMesh(renderToTextureMesh);

	renderToTextureMeshComponent->SetMesh(renderToTextureMesh);
	renderToTextureMeshComponent->SetRelativePosition(Vector3(0.f, -11.f, 0.f));
	renderToTextureMeshComponent->SetRelativeRotation(Vector3(0.f, 0.f, 0.f));
	renderToTextureMeshComponent->SetRelativeScaling(Vector3(5.f, 5.f, 5.f));
}
