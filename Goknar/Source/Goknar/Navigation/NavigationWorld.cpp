#include "NavigationWorld.h"

#include "NavigationMesh.h"
#include "NavigationTreeSerializer.h"

NavigationWorld::NavigationWorld()
{
	navigationMesh_ = new NavigationMesh();
	NavigationTreeSerializer::Deserialize("Navigation/NavigationTree", navigationMesh_->GetNavigationTree());

	pathFinder_ = new PathFinder(&navigationMesh_->GetNavigationTree());
}

NavigationWorld::~NavigationWorld()
{
	delete pathFinder_;
	delete navigationMesh_;
}

void NavigationWorld::BuildFromScene(Scene* scene, const NavMeshSettings& settings)
{
	if (navigationMesh_)
	{
		navigationMesh_->BuildFromScene(scene, settings, true);
	}
}

bool NavigationWorld::FindPath(const Vector3& start, const Vector3& end, std::vector<NavigationPath>& path)
{
	return pathFinder_->FindPath(start, end, path);
}
