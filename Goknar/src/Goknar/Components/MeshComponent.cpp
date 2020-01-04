#include "pch.h"

#include "MeshComponent.h"

#include "Goknar/Engine.h"
#include "Goknar/Mesh.h"

MeshComponent::MeshComponent() : mesh_(nullptr)
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
