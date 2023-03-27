#include "pch.h"

#include "SpriteComponent.h"

#include "Goknar/Model/DynamicMeshInstance.h"
#include "Goknar/Engine.h"
#include "Goknar/Model/2D/SpriteMesh.h"
#include "Goknar/Log.h"

template<class T>
SpriteComponent<T>::SpriteComponent(Component* parent) :
	MeshComponent(parent, new DynamicMeshInstance(this))
{
}

template<class T>
SpriteComponent<T>::~SpriteComponent()
{

}

template<class T>
void SpriteComponent<T>::SetMesh(SpriteMesh* spriteMesh)
{
	meshInstance_->SetMesh(spriteMesh);
}