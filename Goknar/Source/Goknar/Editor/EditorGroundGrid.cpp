#include "pch.h"

#include "EditorGroundGrid.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Scene.h"
#include "Goknar/Components/StaticMeshComponent.h"

constexpr char* vertexShader = R"(
#version 440 core

layout(location = 0) in vec4 color;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

// Transformation matrix is calculated by multiplying  
// world and relative transformation matrices
uniform mat4 worldTransformationMatrix;
uniform mat4 relativeTransformationMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 fragmentPosition;
out vec3 vertexNormal;

void main()
{
	mat4 transformationMatrix = worldTransformationMatrix * relativeTransformationMatrix;
	vec4 fragmentPosition4Channel = vec4(position, 1.f) * transformationMatrix;
	gl_Position = projectionMatrix * viewMatrix * fragmentPosition4Channel;
	vertexNormal = vec3(vec4(normal, 0.f) * transpose(inverse(transformationMatrix)));
	fragmentPosition = vec3(fragmentPosition4Channel);
}
)";

constexpr char* fragmentShader = R"(
#version 440 core

#define PI 3.141592653589793238462643383279

uniform vec3 viewPosition;

in vec3 fragmentPosition;

out vec4 fragmentColor;

vec4 oneGridColor = vec4(0.25f, 0.25f, 0.25f, 1.f);
vec4 oneTenthGridColor = vec4(0.125f, 0.125f, 0.125f, 1.f);
vec4 axisColorX = vec4(0.588235294f, 0.196078431f, 0.196078431f, 1.f);
vec4 axisColorY = vec4(0.196078431f, 0.588235294f, 0.196078431f, 1.f);

float maxGridDistance = 250.f;

void main()
{
	float absoluteDifferenceX = abs(fragmentPosition.x - round(fragmentPosition.x));
	float absoluteDifferenceY = abs(fragmentPosition.y - round(fragmentPosition.y));

	float oneThirdValueX = absoluteDifferenceX / 0.2f;
	float oneThirdValueXRound = round(oneThirdValueX);

	float oneThirdValueY = absoluteDifferenceY / 0.2f;
	float oneThirdValueYRound = round(oneThirdValueY);

	if (abs(fragmentPosition.x) < 0.01f)
	{
		fragmentColor = axisColorX;
	}
	else if (abs(fragmentPosition.y) < 0.01f)
	{
		fragmentColor = axisColorY;
	}
	else if(absoluteDifferenceX < 0.005f || absoluteDifferenceY < 0.005f)
	{
		//vec3 wo = viewPosition - fragmentPosition;
		fragmentColor = oneGridColor;
	}
	else if (abs(oneThirdValueX - oneThirdValueXRound) < 0.01f || 
			 abs(oneThirdValueY - oneThirdValueYRound) < 0.01f)
	{
		fragmentColor = oneTenthGridColor;
	}
	else
	{
		discard;
	}
}
)";

EditorGroundGrid::EditorGroundGrid()
{
	Shader* groundGridMeshShader = new Shader();
	groundGridMeshShader->SetShaderType(ShaderType::SelfContained);
	groundGridMeshShader->SetVertexShaderScript(vertexShader);
	groundGridMeshShader->SetFragmentShaderScript(fragmentShader);

	Material* groundGridMeshMaterial = new Material();
	groundGridMeshMaterial->SetShadingModel(MaterialShadingModel::TwoSided);
	groundGridMeshMaterial->SetShader(groundGridMeshShader);

	StaticMesh* groundGridMesh = new StaticMesh();
	groundGridMesh->AddVertexData(VertexData(Vector3(-1.f, 1.f, 0.f), Vector3(0.f, 0.f, 1.f)));
	groundGridMesh->AddVertexData(VertexData(Vector3(1.f, 1.f, 0.f), Vector3(0.f, 0.f, 1.f)));
	groundGridMesh->AddVertexData(VertexData(Vector3(-1.f, -1.f, 0.f), Vector3(0.f, 0.f, 1.f)));
	groundGridMesh->AddVertexData(VertexData(Vector3(1.f, -1.f, 0.f), Vector3(0.f, 0.f, 1.f)));
	groundGridMesh->AddFace(Face(1, 0, 2));
	groundGridMesh->AddFace(Face(1, 2, 3));
	groundGridMesh->SetMaterial(groundGridMeshMaterial);
	groundGridMesh->Init();

	gridMeshComponent_ = new StaticMeshComponent(this);
	gridMeshComponent_->SetMesh(groundGridMesh);
}

EditorGroundGrid::~EditorGroundGrid()
{
	StaticMesh* groundGridMesh = gridMeshComponent_->GetStaticMesh();
	delete groundGridMesh->GetMaterial()->GetShader();
	delete groundGridMesh->GetMaterial();
	delete groundGridMesh;
}

void EditorGroundGrid::BeginGame()
{
	SetWorldPosition(Vector3::ZeroVector);
	SetWorldScaling(Vector3(1000.f, 1000.f, 1.f));

	SetIsActive(false);
}

void EditorGroundGrid::Init()
{
}
