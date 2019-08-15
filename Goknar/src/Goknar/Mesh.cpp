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
			//layout(location = 1) in vec3 normal;

			uniform mat4 MVP;

			out vec4 vertexNormal;

			void main()
			{
				gl_Position = MVP * vec4(position, 1.0);
				//vertexNormal = MVP * vec4(normal);
			}
		)";

	const char* fragmentBuffer =
		R"(
			#version 330

			out vec4 color;

			in vec4 vertexNormal;

			void main() 
			{
			  color = vec4(1.f, 0.f, 0.f, 1.f);
			}
		)";
	shader_ = new Shader(vertexBuffer, fragmentBuffer);
}

void Mesh::Render() const
{
	Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	Matrix MVP = activeCamera->GetViewingMatrix() * activeCamera->GetProjectionMatrix();
	MVP = MVP * modelMatrix_;

	GLint mVPUniform = glGetUniformLocation(shader_->GetProgramId(), "MVP");
	glUniformMatrix4fv(mVPUniform, 1, GL_FALSE, &MVP[0]);
}
