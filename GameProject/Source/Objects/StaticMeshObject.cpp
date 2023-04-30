#include "StaticMeshObject.h"

#include "Goknar/Components/StaticMeshComponent.h"

StaticMeshObject::StaticMeshObject() : ObjectBase()
{
	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();
}