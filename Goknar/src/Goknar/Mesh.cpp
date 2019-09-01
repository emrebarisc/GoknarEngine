#include "pch.h"

#include "Mesh.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Scene.h"
#include "Goknar/Shader.h"

#include "Managers/CameraManager.h"
#include "Managers/ShaderManager.h"

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
	if (vertices_) delete[] vertices_;
	if (faces_) delete[] faces_;
}

void Mesh::Init()
{
	vertexCount_ = (int)vertices_->size();
	faceCount_ = (int)faces_->size();

	const char* vertexBuffer = engine->GetShaderManager()->GetSceneVertexShader().c_str();
	const char* fragmentBuffer = engine->GetShaderManager()->GetSceneFragmentShader().c_str();
	shader_ = new Shader(vertexBuffer, fragmentBuffer);
}

void Mesh::Render() const
{
	const Scene* scene = engine->GetApplication()->GetMainScene();

	Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	Matrix MVP = activeCamera->GetViewingMatrix() * activeCamera->GetProjectionMatrix();
	MVP = MVP * modelMatrix_;

	shader_->SetMatrix("MVP", &MVP[0]);
	shader_->SetMatrix("modelMatrix", &modelMatrix_[0]);

	const Vector3& cameraPosition = engine->GetCameraManager()->GetActiveCamera()->GetPosition();
	shader_->SetVector3("viewPosition", &cameraPosition.x);

	// Material
	shader_->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT, &scene->GetMaterial(materialId_)->GetAmbientReflectance().x);
	shader_->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE, &scene->GetMaterial(materialId_)->GetDiffuseReflectance().x);
	shader_->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR, &scene->GetMaterial(materialId_)->GetSpecularReflectance().x);
	shader_->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT, scene->GetMaterial(materialId_)->GetPhongExponent());
}
