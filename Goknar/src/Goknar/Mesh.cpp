#include "pch.h"

#include "Mesh.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Scene.h"
#include "Goknar/Renderer/Shader.h"

#include "Managers/CameraManager.h"
#include "Managers/IOManager.h"
#include "Renderer/ShaderBuilder.h"

// TEMP
#include "Renderer/Texture.h"

Mesh::Mesh() :
	material_(0), 
	shader_(0), 
	vertexCount_(0), 
	faceCount_(0)
{
	vertices_ = new VertexArray();
	faces_ = new FaceArray();
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

	if (GetComponentId() == 0)
	{
		std::string vertexBuffer;
		std::string fragmentBuffer;
		IOManager::ReadFile("./Shaders/TextureVertexShader.glsl", vertexBuffer);
		IOManager::ReadFile("./Shaders/TextureFragmentShader.glsl", fragmentBuffer);
		shader_ = new Shader(vertexBuffer.c_str(), fragmentBuffer.c_str());

		Texture* texture = new Texture();
		texture->SetTextureImagePath("Content/Textures/treesBusheShrums.png");
		texture->Init();
		material_->AddTexture(texture);
	
		glActiveTexture(GL_TEXTURE0);
		material_->GetTextures().at(0)->Bind();
		glUniform1i(glGetUniformLocation(shader_->GetProgramId(), "diffuseTexture"), material_->GetTextures().at(0)->GetTextureId());
	}
	else
	{
		const char* vertexBuffer = engine->GetShaderBuilder()->GetDefaultSceneVertexShader().c_str();
		const char* fragmentBuffer = engine->GetShaderBuilder()->GetDefaultSceneFragmentShader().c_str();
		shader_ = new Shader(vertexBuffer, fragmentBuffer);

	}

	engine->AddObjectToRenderer(this);
}

void Mesh::Render() const
{
	const Scene* scene = engine->GetApplication()->GetMainScene();

	Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	
	Matrix transformationMatrix = worldTransformationMatrix_ * relativeTransformationMatrix_;

	shader_->SetMatrix("transformationMatrix", transformationMatrix);
	shader_->SetMatrix("viewMatrix", activeCamera->GetViewingMatrix());
	shader_->SetMatrix("projectionMatrix", activeCamera->GetProjectionMatrix());
	
	const Vector3& cameraPosition = engine->GetCameraManager()->GetActiveCamera()->GetPosition();
	shader_->SetVector3("viewPosition", cameraPosition);

	// Material
	material_->Render(shader_);

	scene->SetShaderDynamicLightUniforms(shader_);
}
