#include "pch.h"

#include "SpriteMesh.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Log.h"
#include "Goknar/Material.h"
#include "Goknar/Scene.h"

#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Texture.h"

SpriteMesh::SpriteMesh() : 
	DynamicMesh(),
	texturePosition_(Vector2(0.f, 0.f), Vector2(1.f, 1.f)),
	material_(nullptr), 
	width_(1.f), 
	height_(1.f)
{
	Scene* scene = engine->GetApplication()->GetMainScene();
	scene->AddDynamicMesh(this);
}

SpriteMesh::SpriteMesh(Material* material) : SpriteMesh()
{
	material_ = material;
	SetMaterial(material_);
}

void SpriteMesh::Init()
{
	NormalizeTexturePosition();

	float sizeRatio = (float)width_ / height_;

	AddVertexData(VertexData(Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(texturePosition_.GetMinX(), texturePosition_.GetMinY())));
	AddVertexData(VertexData(Vector3(sizeRatio, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(texturePosition_.GetMaxX(), texturePosition_.GetMinY())));
	AddVertexData(VertexData(Vector3(0.f, -1.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(texturePosition_.GetMinX(), texturePosition_.GetMaxY())));
	AddVertexData(VertexData(Vector3(sizeRatio, -1.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(texturePosition_.GetMaxX(), texturePosition_.GetMaxY())));

	AddFace(Face(1, 0, 2));
	AddFace(Face(1, 2, 3));

	Mesh::Init();
}

void SpriteMesh::SetTexturePosition(const Rect& texturePosition)
{
	texturePosition_ = texturePosition;
}

void SpriteMesh::NormalizeTexturePosition()
{
	const Texture* texture = material_->GetShader()->GetTextures()->at(0);
	float textureWidth = (float)texture->GetWidth();
	float textureHeight = (float)texture->GetHeight();

	Vector2 min = texturePosition_.GetMin();
	min.x /= textureWidth;
	min.y /= textureHeight;

	Vector2 max = texturePosition_.GetMax();
	max.x /= textureWidth;
	max.y /= textureHeight;

	texturePosition_ = Rect(min, max);
}
