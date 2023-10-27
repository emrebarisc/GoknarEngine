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

#define MAX_DISTANCE 100

void main()
{
	float thickness = 5.f;

	float distanceFromViewPosition = length(viewPosition - fragmentPosition);

	float distanceMultiplier = clamp(distanceFromViewPosition / MAX_DISTANCE, 0.025f, 1.f);

	thickness *= 5.f * distanceMultiplier;

	float absoluteDifferenceX = abs(fragmentPosition.x - round(fragmentPosition.x));
	float absoluteDifferenceY = abs(fragmentPosition.y - round(fragmentPosition.y));

	float oneThirdValueX = absoluteDifferenceX / 0.1f;
	float oneThirdValueXRound = round(oneThirdValueX);

	float oneThirdValueY = absoluteDifferenceY / 0.1f;
	float oneThirdValueYRound = round(oneThirdValueY);

	if (abs(fragmentPosition.x) < (0.01f * thickness))
	{
		fragmentColor = axisColorY;
	}
	else if (abs(fragmentPosition.y) < (0.01f * thickness))
	{
		fragmentColor = axisColorX;
	}
	else if(MAX_DISTANCE < distanceFromViewPosition)
	{
		discard;
	}
	else if(absoluteDifferenceX < (0.01f * thickness) || absoluteDifferenceY <  (0.01f * thickness))
	{
		//vec3 wo = viewPosition - fragmentPosition;
		fragmentColor = oneGridColor;
	}
	else if (distanceFromViewPosition < (MAX_DISTANCE * 0.5f) &&
			(abs(oneThirdValueX - oneThirdValueXRound) < (0.05f * thickness) || 
			 abs(oneThirdValueY - oneThirdValueYRound) < (0.05f * thickness)))
	{
		fragmentColor = oneTenthGridColor;
	}
	else
	{
		discard;
	}
}
)";

EditorGroundGrid::EditorGroundGrid() : ObjectBase()
{
	Shader* groundGridMeshShader = new Shader();
	groundGridMeshShader->SetShaderType(ShaderType::SelfContained);
	groundGridMeshShader->SetVertexShaderScript(vertexShader);
	groundGridMeshShader->SetFragmentShaderScript(fragmentShader);

	groundGridMeshMaterial_ = new Material();
	groundGridMeshMaterial_->SetShadingModel(MaterialShadingModel::TwoSided);
	groundGridMeshMaterial_->SetShader(groundGridMeshShader);

	groundGridMesh_ = new StaticMesh();
	groundGridMesh_->AddVertexData(VertexData(Vector3(-1.f, 1.f, 0.01f), Vector3(0.f, 0.f, 1.f)));
	groundGridMesh_->AddVertexData(VertexData(Vector3(1.f, 1.f, 0.01f), Vector3(0.f, 0.f, 1.f)));
	groundGridMesh_->AddVertexData(VertexData(Vector3(-1.f, -1.f, 0.01f), Vector3(0.f, 0.f, 1.f)));
	groundGridMesh_->AddVertexData(VertexData(Vector3(1.f, -1.f, 0.01f), Vector3(0.f, 0.f, 1.f)));
	groundGridMesh_->AddFace(Face(1, 0, 2));
	groundGridMesh_->AddFace(Face(1, 2, 3));
	groundGridMesh_->SetMaterial(groundGridMeshMaterial_);
	groundGridMesh_->Init();

	gridMeshComponent_ = AddSubComponent<StaticMeshComponent>();
	gridMeshComponent_->SetMesh(groundGridMesh_);
}

EditorGroundGrid::~EditorGroundGrid()
{
	delete groundGridMeshMaterial_;
	delete groundGridMesh_;
}

void EditorGroundGrid::BeginGame()
{
	SetWorldPosition(Vector3::ZeroVector);
	SetWorldScaling(Vector3(1000.f, 1000.f, 1.f));
}

void EditorGroundGrid::Init()
{
	ObjectBase::Init();
}
