#include "pch.h"

#include "Mesh.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/Renderer.h"
#include "Goknar/Shader.h"

#include "Goknar/Managers/CameraManager.h"

Mesh::Mesh() : modelMatrix_(Matrix::IdentityMatrix), materialId_(0), indexBufferId_(0), shader_(0), vertexBufferId_(0), vertexArrayId_(0)
{
	engine->GetRenderer()->AddObjectToRenderer(this);
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &vertexArrayId_);
	glDeleteBuffers(1, &vertexBufferId_);
	glDeleteBuffers(1, &indexBufferId_);
}

void Mesh::Init()
{
	glGenVertexArrays(1, &vertexArrayId_);
	glGenBuffers(1, &vertexBufferId_);
	glGenBuffers(1, &indexBufferId_);

	glBindVertexArray(vertexArrayId_);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId_);
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(vertices_[0]), &vertices_[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces_.size() * sizeof(faces_[0]), &faces_[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices_[0]), nullptr);

	// TODO: REMOVE SHADER OPERATION HERE
	const char* vertexBuffer =
		R"(
			#version 330 core

			uniform float time;

			uniform mat4 MVP;

			layout(location = 0) in vec3 position;
			
			void main()
			{
				gl_Position = MVP * vec4(position, 1.0);
			}
		)";

	const char* fragmentBuffer =
		R"(
			#version 330

			out vec4 color;

			void main() 
			{
			  color = vec4(0.2, 0.8, 0.2, 1.0);
			}
		)";

	/*char* vertexBuffer = nullptr;
	if (IOManager::GetFileRawText("C:/Users/emreb/Desktop/DefaultShadersDefaultVertexShader.glsl", vertexBuffer))
	{

	}

	char* fragmentBuffer = nullptr;
	if (IOManager::GetFileRawText("C:/Users/emreb/Desktop/DefaultShadersDefaultFragmentShader.glsl", fragmentBuffer))
	{

	}*/

	shader_ = new Shader(vertexBuffer, fragmentBuffer);
}

void Mesh::Render() const
{
	glUseProgram(shader_->GetProgramId());

	Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	Matrix MVP = activeCamera->GetViewingMatrix() * activeCamera->GetProjectionMatrix();
	MVP = MVP * modelMatrix_;
	glUseProgram(shader_->GetProgramId());
	GLint uniMvp = glGetUniformLocation(shader_->GetProgramId(), "MVP");

	glUniformMatrix4fv(uniMvp, 1, GL_FALSE, &MVP[0]);

	//glBindVertexArray(vertexArrayId_);
	glDrawElements(GL_TRIANGLES, faces_.size(), GL_UNSIGNED_INT, 0);
	//glBindVertexArray(0);
}
