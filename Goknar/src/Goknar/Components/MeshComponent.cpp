#include "pch.h"

#include "MeshComponent.h"

#include "Goknar/Engine.h"
#include "Goknar/Model/Mesh.h"

MeshComponent::MeshComponent(const ObjectBase* parent) : Component(parent), mesh_(nullptr)
{
}

MeshComponent::~MeshComponent()
{

}

void MeshComponent::SetMesh(Mesh* mesh)
{
	mesh_ = mesh;
	engine->AddObjectToRenderer(mesh);
}
