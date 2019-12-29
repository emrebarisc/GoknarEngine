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

	//if (GetComponentId() == 0)
	//{	
	//	glActiveTexture(GL_TEXTURE0);
	//	material_->GetTextures().at(0)->Bind();
	//	glUniform1i(glGetUniformLocation(shader_->GetProgramId(), SHADER_VARIABLE_NAMES::TEXTURE::DIFFUSE), material_->GetTextures().at(0)->GetTextureObjectId());
	//}

	engine->AddObjectToRenderer(this);
}

void Mesh::Render() const
{
	material_->Render(worldTransformationMatrix_ * relativeTransformationMatrix_);
}
