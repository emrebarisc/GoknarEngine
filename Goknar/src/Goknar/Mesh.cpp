#include "pch.h"

#include "Mesh.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Scene.h"
#include "Goknar/Shader.h"

#include "Managers/CameraManager.h"
#include "Managers/ShaderBuilder.h"

Mesh::Mesh() :
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

	//if (GetComponentId() == 0)
	//{
	//	for (int i = 0; i < vertexCount_; i++)
	//	{
	//		vertices_->at(i).position = relativeTransformationMatrix_ * Vector4(vertices_->at(i).position, 1);
	//	}
	//	relativeTransformationMatrix_ = Matrix::IdentityMatrix;
	//}

	const char* vertexBuffer = engine->GetShaderBuilder()->GetSceneVertexShader().c_str();
	const char* fragmentBuffer = engine->GetShaderBuilder()->GetSceneFragmentShader().c_str();
	shader_ = new Shader(vertexBuffer, fragmentBuffer);
}

void Mesh::Render() /*const*/
{
	static bool isThatOneTimeIt = true;

	const Scene* scene = engine->GetApplication()->GetMainScene();

	Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	
	Matrix modelMatrix = worldTransformationMatrix_ * relativeTransformationMatrix_;

	Matrix MVP = modelMatrix;
	MVP = MVP * activeCamera->GetViewingMatrix();
	MVP = MVP * activeCamera->GetProjectionMatrix();

	shader_->SetMatrix("MVP", MVP);
	shader_->SetMatrix("relativeTransformationMatrix", relativeTransformationMatrix_);
	shader_->SetMatrix("worldTransformationMatrix", worldTransformationMatrix_);
	shader_->SetMatrix("viewMatrix", activeCamera->GetViewingMatrix());
	shader_->SetMatrix("projectionMatrix", activeCamera->GetProjectionMatrix());

	const Vector3& cameraPosition = engine->GetCameraManager()->GetActiveCamera()->GetPosition();
	shader_->SetVector3("viewPosition", cameraPosition);

	// Material
	shader_->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::AMBIENT, scene->GetMaterial(materialId_)->GetAmbientReflectance());
	shader_->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::DIFFUSE, scene->GetMaterial(materialId_)->GetDiffuseReflectance());
	shader_->SetVector3(SHADER_VARIABLE_NAMES::MATERIAL::SPECULAR, scene->GetMaterial(materialId_)->GetSpecularReflectance());
	shader_->SetFloat(SHADER_VARIABLE_NAMES::MATERIAL::PHONG_EXPONENT, scene->GetMaterial(materialId_)->GetPhongExponent());

	scene->SetShaderDynamicLightUniforms(shader_);
}
