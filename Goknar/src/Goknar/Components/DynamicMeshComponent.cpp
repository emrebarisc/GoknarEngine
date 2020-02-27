#include "pch.h"

#include "DynamicMeshComponent.h"

#include "Goknar/Model/MeshInstance.h"
#include "Goknar/Model/DynamicMeshInstance.h"

DynamicMeshComponent::DynamicMeshComponent(ObjectBase* parent) : 
	MeshComponent(parent)
{
	meshInstance_ = new DynamicMeshInstance(this);
}

DynamicMeshComponent::~DynamicMeshComponent()
{

}