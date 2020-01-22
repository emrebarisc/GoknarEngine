#include "pch.h"

#include "Mesh.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Scene.h"
#include "Goknar/Renderer/Shader.h"

#include "Goknar/Managers/CameraManager.h"
#include "Goknar/IO/IOManager.h"
#include "Goknar/Renderer/ShaderBuilder.h"

Mesh::Mesh() :
	material_(0), 
	vertexCount_(0), 
	faceCount_(0),
	baseVertex_(0),
	vertexStartingIndex_(0)
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
}
