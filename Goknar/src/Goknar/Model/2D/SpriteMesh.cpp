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
	textureCoordinate_(Vector2(0.f, 0.f), Vector2(1.f, 1.f)),
	material_(nullptr), 
	width_(1.f), 
	height_(1.f)
{
	// TODO: For every sprite a mesh is created.
	Scene* scene = engine->GetApplication()->GetMainScene();
	scene->AddDynamicMesh(this);
}


SpriteMesh::SpriteMesh(Material* material) :
	SpriteMesh()
{
	material_ = material;
	SetMaterial(material_);
}

void SpriteMesh::Init()
{
	NormalizeTexturePosition();

	float minX = textureCoordinate_.GetMinX();
	float minY = textureCoordinate_.GetMinY();
	float maxX = textureCoordinate_.GetMaxX();
	float maxY = textureCoordinate_.GetMaxY();

	AddVertexData(VertexData(Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(minX, minY)));
	AddVertexData(VertexData(Vector3(width_, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(maxX, minY)));
	AddVertexData(VertexData(Vector3(0.f, -height_, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(minX, maxY)));
	AddVertexData(VertexData(Vector3(width_, -height_, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(maxX, maxY)));

	AddFace(Face(1, 0, 2));
	AddFace(Face(1, 2, 3));

	Mesh::Init();
}

void SpriteMesh::SetTextureCoordinate(const Rect& textureCoordinate)
{
	textureCoordinate_ = textureCoordinate;
}

void SpriteMesh::UpdateSpriteMeshVertexData()
{
	NormalizeTexturePosition();

	float minX = textureCoordinate_.GetMinX();
	float minY = textureCoordinate_.GetMinY();
	float maxX = textureCoordinate_.GetMaxX();
	float maxY = textureCoordinate_.GetMaxY();

	UpdateVertexDataAt(0, VertexData(Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(minX, minY)));
	UpdateVertexDataAt(1, VertexData(Vector3(width_, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(maxX, minY)));
	UpdateVertexDataAt(2, VertexData(Vector3(0.f, -height_, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(minX, maxY)));
	UpdateVertexDataAt(3, VertexData(Vector3(width_, -height_, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(maxX, maxY)));
}

void SpriteMesh::NormalizeTexturePosition()
{
	// TODO: Change getting texture with index to a hast table where it is gotten with GetShader()->GetDiffuseTexture()
	const Texture* texture = material_->GetShader()->GetTextures()->at(0);
	float textureWidth = (float)texture->GetWidth();
	float textureHeight = (float)texture->GetHeight();

	Vector2 min = textureCoordinate_.GetMin();
	min.x /= textureWidth;
	min.y /= textureHeight;

	Vector2 max = textureCoordinate_.GetMax();
	max.x /= textureWidth;
	max.y /= textureHeight;

	textureCoordinate_ = Rect(min, max);
}
