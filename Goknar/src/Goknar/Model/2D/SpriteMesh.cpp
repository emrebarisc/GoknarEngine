#include "pch.h"

#include "SpriteMesh.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Log.h"
#include "Goknar/Material.h"
#include "Goknar/Scene.h"

SpriteMesh::SpriteMesh() : Mesh(), material_(nullptr), width_(1), height_(1)
{
	Scene* scene = engine->GetApplication()->GetMainScene();
	scene->AddMesh(this);
}

SpriteMesh::SpriteMesh(Material* material) : SpriteMesh()
{
	material_ = material;
	SetMaterial(material_);
}

void SpriteMesh::Init()
{
	float sizeRatio = (float)width_ / height_;

	AddVertexData(VertexData(Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(0.f, 0.f)));
	AddVertexData(VertexData(Vector3(sizeRatio, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(1.f, 0.f)));
	AddVertexData(VertexData(Vector3(0.f, -1.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(0.f, 1.f)));
	AddVertexData(VertexData(Vector3(sizeRatio, -1.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(1.f, 1.f)));

	AddFace(Face(1, 0, 2));
	AddFace(Face(1, 2, 3));

	Mesh::Init();
}
