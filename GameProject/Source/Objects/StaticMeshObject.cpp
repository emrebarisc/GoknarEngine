#include "StaticMeshObject.h"

#include "Goknar/Components/StaticMeshComponent.h"

StaticMeshObject::StaticMeshObject() : ObjectBase()
{
	staticMeshComponent = AddSubComponent<StaticMeshComponent>();
}