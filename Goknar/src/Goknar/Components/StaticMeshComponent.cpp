#include "pch.h"

#include "StaticMeshComponent.h"

#include "Goknar/Model/StaticMeshInstance.h"

StaticMeshComponent::StaticMeshComponent(ObjectBase* parent) : 
	MeshComponent(parent)	
{
	meshInstance_ = new StaticMeshInstance(this);
}

StaticMeshComponent::~StaticMeshComponent()
{

}