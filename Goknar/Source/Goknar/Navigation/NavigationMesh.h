#ifndef __NAVIGATION_MESH__
#define __NAVIGATION_MESH__

#include "Goknar/Core.h"
#include "Goknar/Navigation/NavigationTree.h"
#include "Goknar/Navigation/NavigationTypes.h"

class Scene;

class GOKNAR_API NavigationMesh
{
public:
	NavigationMesh() = default;
	~NavigationMesh() = default;

	void Clear();
	void BuildFromScene(Scene* scene, const NavMeshSettings& settings, bool removeNavigationTreeObjects = false);

	const NavigationTree& GetNavigationTree() const
	{
		return navigationTree_;
	}

	NavigationTree& GetNavigationTree()
	{
		return navigationTree_;
	}

private:
	NavigationTree navigationTree_;
};

#endif
