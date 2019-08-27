#include "pch.h"

#include "Mesh.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Shader.h"

#include "Goknar/Managers/CameraManager.h"

Mesh::Mesh() :
	modelMatrix_(Matrix::IdentityMatrix), 
	materialId_(0), 
	shader_(0), 
	vertexCount_(0), 
	faceCount_(0)
{
	vertices_ = new VertexArray();
	faces_ = new FaceArray();

	engine->AddObjectToRenderer(this);
}

Mesh::~Mesh()
{
}

void Mesh::Init()
{
	vertexCount_ = vertices_->size();
	faceCount_ = faces_->size();

	const char* vertexBuffer =
		R"(
			#version 330 core

			layout(location = 0) in vec3 position;
			layout(location = 1) in vec3 normal;

			uniform mat4 MVP;

uniform mat4 modelMatrix;

out vec3 fragmentPosition;

			out vec3 vertexNormal;

			void main()
			{
				gl_Position = MVP * vec4(position, 1.f);
				vertexNormal = mat3(transpose(inverse(modelMatrix))) * normal;
fragmentPosition = vec3(modelMatrix * vec4(position, 1.f));
			}
		)";

	const char* fragmentBuffer =
		R"(
			#version 330

			out vec4 color;

in vec3 fragmentPosition;
			
uniform vec3 pointLightPosition;
uniform vec3 pointLightIntensity;

uniform vec3 viewPosition;

			in vec3 vertexNormal;

			void main() 
			{
// Ambient
vec3 ambient = vec3(0.15f, 0.15f, 0.15f);

// Diffuse
vec3 diffuseReflectance = vec3(1.f, 1.f, 1.f);

vec3 wi = pointLightPosition - fragmentPosition;
float wiLength = length(wi);
wi /= wiLength;

float cosThetaPrime = max(0.f, dot(wi, vertexNormal));

vec3 diffuse = diffuseReflectance * cosThetaPrime * pointLightIntensity / (wiLength * wiLength);


// Specular
vec3 specularReflectance = vec3(1000.f, 1000.f, 1000.f);
float phongExponent = 50.f;

vec3 wo = viewPosition - fragmentPosition;
float woLength = length(wo);
wo /= woLength;

vec3 halfVector = (wi + wo) / (wiLength + woLength);

float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0, dot(vertexNormal, halfVector)), phongExponent);
vec3 specular = specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

			  color = vec4((ambient + diffuse + specular), 1.f);
			}
		)";
	shader_ = new Shader(vertexBuffer, fragmentBuffer);
}

void Mesh::Render() const
{
	Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	Matrix MVP = activeCamera->GetViewingMatrix() * activeCamera->GetProjectionMatrix();
	MVP = MVP * modelMatrix_;

	shader_->SetMatrix("MVP", &MVP[0]);
	shader_->SetMatrix("modelMatrix", &modelMatrix_[0]);

	static float theta = 0.f;
	static float radius = 10.f;
	theta += 0.00025f;
	Vector3 pointLightPosition = Vector3(radius * cos(theta), radius * sin(theta), 10.f);
	shader_->SetVector3("pointLightPosition", &pointLightPosition.x);

	Vector3 pointLightIntensity(100.f, 100.f, 100.f);
	shader_->SetVector3("pointLightIntensity", &pointLightIntensity.x);

	const Vector3& cameraPosition = engine->GetCameraManager()->GetActiveCamera()->GetPosition();
	shader_->SetVector3("viewPosition", &cameraPosition.x);

	shader_->SetVector3("ASDASDASD", &cameraPosition.x);
}
