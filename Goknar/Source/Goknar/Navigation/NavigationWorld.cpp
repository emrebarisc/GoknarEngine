#include "NavigationWorld.h"

#include "NavigationTree.h"
#include "NavigationTreeSerializer.h"

NavigationWorld::NavigationWorld()
{
	navigationTree_ = new NavigationTree();
	NavigationTreeSerializer::Deserialize("Navigation/NavigationTree", *navigationTree_);

	pathFinder_ = new PathFinder(navigationTree_);
}

NavigationWorld::~NavigationWorld()
{
	delete pathFinder_;
	delete navigationTree_;
}

void NavigationWorld::BuildFromScene(const Scene* scene, const NavMeshSettings& settings)
{
}

bool NavigationWorld::FindPath(const Vector3& start, const Vector3& end, std::vector<NavigationPath>& path)
{
	return pathFinder_->FindPath(start, end, path);
}
