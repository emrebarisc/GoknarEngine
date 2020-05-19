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

	Vector3 position1, position2, position3, position4;
	Vector3 normal1, normal2, normal3, normal4;

	AppType applicationType = engine->GetApplication()->GetAppType();
	switch (applicationType)
	{
	case AppType::Application2D:
	{
		switch (pivotPoint_)
		{
		case SPRITE_PIVOT_POINT::TOP_LEFT:
			position1 = Vector3(0.f, 0.f, 0.f);
			position2 = Vector3(width_, 0.f, 0.f);
			position3 = Vector3(0.f, -height_, 0.f);
			position4 = Vector3(width_, -height_, 0.f);
			break;
		case SPRITE_PIVOT_POINT::TOP_MIDDLE:
			position1 = Vector3(-width_ * 0.5f, 0.f, 0.f);
			position2 = Vector3(width_ * 0.5f, 0.f, 0.f);
			position3 = Vector3(-width_ * 0.5f, -height_, 0.f);
			position4 = Vector3(width_ * 0.5f, -height_, 0.f);
			break;
		case SPRITE_PIVOT_POINT::TOP_RIGHT:
			position1 = Vector3(-width_, 0.f, 0.f);
			position2 = Vector3(0.f, 0.f, 0.f);
			position3 = Vector3(-width_, -height_, 0.f);
			position4 = Vector3(0.f, -height_, 0.f);
			break;
		case SPRITE_PIVOT_POINT::MIDDLE_LEFT:
			position1 = Vector3(0.f, height_ * 0.5f, 0.f);
			position2 = Vector3(width_, height_ * 0.5f, 0.f);
			position3 = Vector3(0.f, -height_ * 0.5f, 0.f);
			position4 = Vector3(width_, -height_ * 0.5f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::MIDDLE_MIDDLE:
			position1 = Vector3(-width_ * 0.5f, height_ * 0.5f, 0.f);
			position2 = Vector3(width_ * 0.5f, height_ * 0.5f, 0.f);
			position3 = Vector3(-width_ * 0.5f, -height_ * 0.5f, 0.f);
			position4 = Vector3(width_ * 0.5f, -height_ * 0.5f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::MIDDLE_RIGHT:
			position1 = Vector3(-width_, height_ * 0.5f, 0.f);
			position2 = Vector3(0.f, height_ * 0.5f, 0.f);
			position3 = Vector3(-width_, -height_ * 0.5f, 0.f);
			position4 = Vector3(0.f, -height_ * 0.5f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::BOTTOM_LEFT:
			position1 = Vector3(0.f, height_, 0.f);
			position2 = Vector3(width_, height_, 0.f);
			position3 = Vector3(0.f, 0.f, 0.f);
			position4 = Vector3(width_, 0.f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::BOTTOM_MIDDLE:
			position1 = Vector3(-width_ * 0.5f, height_, 0.f);
			position2 = Vector3(width_ * 0.5f, height_, 0.f);
			position3 = Vector3(-width_ * 0.5f, 0.f, 0.f);
			position4 = Vector3(width_ * 0.5f, 0.f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::BOTTOM_RIGHT:
			position1 = Vector3(-width_, height_, 0.f);
			position2 = Vector3(0.f, height_, 0.f);
			position3 = Vector3(-width_, 0.f, 0.f);
			position4 = Vector3(0.f, 0.f, 0.f);
			break;
		default:
			break;
		}

		normal1 = Vector3(0.f, 0.f, 1.f);
		normal2 = Vector3(0.f, 0.f, 1.f);
		normal3 = Vector3(0.f, 0.f, 1.f);
		normal4 = Vector3(0.f, 0.f, 1.f);
		break;
	}
	case AppType::Application3D:
	{
		switch (pivotPoint_)
		{
		case SPRITE_PIVOT_POINT::TOP_LEFT:
			position1 = Vector3(0.f, 0.f, 0.f);
			position2 = Vector3(width_, 0.f, 0.f);
			position3 = Vector3(0.f, 0.f, -height_);
			position4 = Vector3(width_, 0.f, -height_);
			break;
		case SPRITE_PIVOT_POINT::TOP_MIDDLE:
			position1 = Vector3(-width_ * 0.5f, 0.f, 0.f);
			position2 = Vector3(width_ * 0.5f, 0.f, 0.f);
			position3 = Vector3(-width_ * 0.5f, 0.f, -height_);
			position4 = Vector3(width_ * 0.5f, 0.f, -height_);
			break;
		case SPRITE_PIVOT_POINT::TOP_RIGHT:
			position1 = Vector3(-width_, 0.f, 0.f);
			position2 = Vector3(0.f, 0.f, 0.f);
			position3 = Vector3(-width_, 0.f, -height_);
			position4 = Vector3(0.f, 0.f, -height_);
			break;
		case SPRITE_PIVOT_POINT::MIDDLE_LEFT:
			position1 = Vector3(0.f, 0.f, height_ * 0.5f);
			position2 = Vector3(width_, 0.f, height_ * 0.5f);
			position3 = Vector3(0.f, 0.f, -height_ * 0.5f);
			position4 = Vector3(width_, 0.f, -height_ * 0.5f);
			break;
		case SPRITE_PIVOT_POINT::MIDDLE_MIDDLE:
			position1 = Vector3(-width_ * 0.5f, 0.f, height_ * 0.5f);
			position2 = Vector3(width_ * 0.5f, 0.f, height_ * 0.5f);
			position3 = Vector3(-width_ * 0.5f, 0.f, -height_ * 0.5f);
			position4 = Vector3(width_ * 0.5f, 0.f, -height_ * 0.5f);
			break;
		case SPRITE_PIVOT_POINT::MIDDLE_RIGHT:
			position1 = Vector3(-width_, 0.f, height_ * 0.5f);
			position2 = Vector3(0.f, 0.f, height_ * 0.5f);
			position3 = Vector3(-width_, 0.f, -height_ * 0.5f);
			position4 = Vector3(0.f, 0.f, -height_ * 0.5f);
			break;
		case SPRITE_PIVOT_POINT::BOTTOM_LEFT:
			position1 = Vector3(0.f, 0.f, height_);
			position2 = Vector3(width_, 0.f, height_);
			position3 = Vector3(0.f, 0.f, 0.f);
			position4 = Vector3(width_, 0.f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::BOTTOM_MIDDLE:
			position1 = Vector3(-width_ * 0.5f, 0.f, height_);
			position2 = Vector3(width_ * 0.5f, 0.f, height_);
			position3 = Vector3(-width_ * 0.5f, 0.f, 0.f);
			position4 = Vector3(width_ * 0.5f, 0.f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::BOTTOM_RIGHT:
			position1 = Vector3(-width_, 0.f, height_);
			position2 = Vector3(0.f, 0.f, height_);
			position3 = Vector3(-width_, 0.f, 0.f);
			position4 = Vector3(0.f, 0.f, 0.f);
			break;
		default:
			break;
		}
		normal1 = Vector3(0.f, 1.f, 0.f);
		normal2 = Vector3(0.f, 1.f, 0.f);
		normal3 = Vector3(0.f, 1.f, 0.f);
		normal4 = Vector3(0.f, 1.f, 0.f);
		break;
	}
	default:
		break;
	}

	AddVertexData(VertexData(position1, normal1, Vector4::ZeroVector, Vector2(minX, minY)));
	AddVertexData(VertexData(position2, normal2, Vector4::ZeroVector, Vector2(maxX, minY)));
	AddVertexData(VertexData(position3, normal3, Vector4::ZeroVector, Vector2(minX, maxY)));
	AddVertexData(VertexData(position4, normal4, Vector4::ZeroVector, Vector2(maxX, maxY)));

	AddFace(Face(1, 0, 2));
	AddFace(Face(1, 2, 3));

	Mesh::Init();
}

void SpriteMesh::SetTextureCoordinate(const Rect& textureCoordinate)
{
	textureCoordinate_ = textureCoordinate;
	SetSize(abs(textureCoordinate.GetMaxX() - textureCoordinate.GetMinX()), abs(textureCoordinate.GetMaxY() - textureCoordinate.GetMinY()));
}

void SpriteMesh::UpdateSpriteMeshVertexData()
{
	NormalizeTexturePosition();

	float minX = textureCoordinate_.GetMinX();
	float minY = textureCoordinate_.GetMinY();
	float maxX = textureCoordinate_.GetMaxX();
	float maxY = textureCoordinate_.GetMaxY();

	Vector3 position1, position2, position3, position4;
	Vector3 normal1, normal2, normal3, normal4;

	AppType applicationType = engine->GetApplication()->GetAppType();
	switch (applicationType)
	{
	case AppType::Application2D:
		switch (pivotPoint_)
		{
		case SPRITE_PIVOT_POINT::TOP_LEFT:
			position1 = Vector3(0.f, 0.f, 0.f);
			position2 = Vector3(width_, 0.f, 0.f);
			position3 = Vector3(0.f, -height_, 0.f);
			position4 = Vector3(width_, -height_, 0.f);
			break;
		case SPRITE_PIVOT_POINT::TOP_MIDDLE:
			position1 = Vector3(-width_ * 0.5f, 0.f, 0.f);
			position2 = Vector3(width_ * 0.5f, 0.f, 0.f);
			position3 = Vector3(-width_ * 0.5f, -height_, 0.f);
			position4 = Vector3(width_ * 0.5f, -height_, 0.f);
			break;
		case SPRITE_PIVOT_POINT::TOP_RIGHT:
			position1 = Vector3(-width_, 0.f, 0.f);
			position2 = Vector3(0.f, 0.f, 0.f);
			position3 = Vector3(-width_, -height_, 0.f);
			position4 = Vector3(0.f, -height_, 0.f);
			break;
		case SPRITE_PIVOT_POINT::MIDDLE_LEFT:
			position1 = Vector3(0.f, height_ * 0.5f, 0.f);
			position2 = Vector3(width_, height_ * 0.5f, 0.f);
			position3 = Vector3(0.f, -height_ * 0.5f, 0.f);
			position4 = Vector3(width_, -height_ * 0.5f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::MIDDLE_MIDDLE:
			position1 = Vector3(-width_ * 0.5f, height_ * 0.5f, 0.f);
			position2 = Vector3(width_ * 0.5f, height_ * 0.5f, 0.f);
			position3 = Vector3(-width_ * 0.5f, -height_ * 0.5f, 0.f);
			position4 = Vector3(width_ * 0.5f, -height_ * 0.5f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::MIDDLE_RIGHT:
			position1 = Vector3(-width_, height_ * 0.5f, 0.f);
			position2 = Vector3(0.f, height_ * 0.5f, 0.f);
			position3 = Vector3(-width_, -height_ * 0.5f, 0.f);
			position4 = Vector3(0.f, -height_ * 0.5f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::BOTTOM_LEFT:
			position1 = Vector3(0.f, height_, 0.f);
			position2 = Vector3(width_, height_, 0.f);
			position3 = Vector3(0.f, 0.f, 0.f);
			position4 = Vector3(width_, 0.f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::BOTTOM_MIDDLE:
			position1 = Vector3(-width_ * 0.5f, height_, 0.f);
			position2 = Vector3(width_ * 0.5f, height_, 0.f);
			position3 = Vector3(-width_ * 0.5f, 0.f, 0.f);
			position4 = Vector3(width_ * 0.5f, 0.f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::BOTTOM_RIGHT:
			position1 = Vector3(-width_, height_, 0.f);
			position2 = Vector3(0.f, height_, 0.f);
			position3 = Vector3(-width_, 0.f, 0.f);
			position4 = Vector3(0.f, 0.f, 0.f);
			break;
		default:
			break;
		}

		normal1 = Vector3(0.f, 0.f, 1.f);
		normal2 = Vector3(0.f, 0.f, 1.f);
		normal3 = Vector3(0.f, 0.f, 1.f);
		normal4 = Vector3(0.f, 0.f, 1.f);
		break;
	case AppType::Application3D:
		switch (pivotPoint_)
		{
		case SPRITE_PIVOT_POINT::TOP_LEFT:
			position1 = Vector3(0.f, 0.f, 0.f);
			position2 = Vector3(width_, 0.f, 0.f);
			position3 = Vector3(0.f, 0.f, -height_);
			position4 = Vector3(width_, 0.f, -height_);
			break;
		case SPRITE_PIVOT_POINT::TOP_MIDDLE:
			position1 = Vector3(-width_ * 0.5f, 0.f, 0.f);
			position2 = Vector3(width_ * 0.5f, 0.f, 0.f);
			position3 = Vector3(-width_ * 0.5f, 0.f, -height_);
			position4 = Vector3(width_ * 0.5f, 0.f, -height_);
			break;
		case SPRITE_PIVOT_POINT::TOP_RIGHT:
			position1 = Vector3(-width_, 0.f, 0.f);
			position2 = Vector3(0.f, 0.f, 0.f);
			position3 = Vector3(-width_, 0.f, -height_);
			position4 = Vector3(0.f, 0.f, -height_);
			break;
		case SPRITE_PIVOT_POINT::MIDDLE_LEFT:
			position1 = Vector3(0.f, 0.f, height_ * 0.5f);
			position2 = Vector3(width_, 0.f, height_ * 0.5f);
			position3 = Vector3(0.f, 0.f, -height_ * 0.5f);
			position4 = Vector3(width_, 0.f, -height_ * 0.5f);
			break;
		case SPRITE_PIVOT_POINT::MIDDLE_MIDDLE:
			position1 = Vector3(-width_ * 0.5f, 0.f, height_ * 0.5f);
			position2 = Vector3(width_ * 0.5f, 0.f, height_ * 0.5f);
			position3 = Vector3(-width_ * 0.5f, 0.f, -height_ * 0.5f);
			position4 = Vector3(width_ * 0.5f, 0.f, -height_ * 0.5f);
			break;
		case SPRITE_PIVOT_POINT::MIDDLE_RIGHT:
			position1 = Vector3(-width_, 0.f, height_ * 0.5f);
			position2 = Vector3(0.f, 0.f, height_ * 0.5f);
			position3 = Vector3(-width_, 0.f, -height_ * 0.5f);
			position4 = Vector3(0.f, 0.f, -height_ * 0.5f);
			break;
		case SPRITE_PIVOT_POINT::BOTTOM_LEFT:
			position1 = Vector3(0.f, 0.f, height_);
			position2 = Vector3(width_, 0.f, height_);
			position3 = Vector3(0.f, 0.f, 0.f);
			position4 = Vector3(width_, 0.f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::BOTTOM_MIDDLE:
			position1 = Vector3(-width_ * 0.5f, 0.f, height_);
			position2 = Vector3(width_ * 0.5f, 0.f, height_);
			position3 = Vector3(-width_ * 0.5f, 0.f, 0.f);
			position4 = Vector3(width_ * 0.5f, 0.f, 0.f);
			break;
		case SPRITE_PIVOT_POINT::BOTTOM_RIGHT:
			position1 = Vector3(-width_, 0.f, height_);
			position2 = Vector3(0.f, 0.f, height_);
			position3 = Vector3(-width_, 0.f, 0.f);
			position4 = Vector3(0.f, 0.f, 0.f);
			break;
		default:
			break;
		}
	}

	UpdateVertexDataAt(0, VertexData(position1, normal1, Vector4::ZeroVector, Vector2(minX, minY)));
	UpdateVertexDataAt(1, VertexData(position2, normal2, Vector4::ZeroVector, Vector2(maxX, minY)));
	UpdateVertexDataAt(2, VertexData(position3, normal3, Vector4::ZeroVector, Vector2(minX, maxY)));
	UpdateVertexDataAt(3, VertexData(position4, normal4, Vector4::ZeroVector, Vector2(maxX, maxY)));
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
