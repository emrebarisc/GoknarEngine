#ifndef __NAVIGATION_WORLD__
#define __NAVIGATION_WORLD__

#include "Goknar/Core.h"

#include "Goknar/Math/GoknarMath.h"

#include "PathFinder.h"

class Scene;

struct NavMeshSettings;

class NavigationMesh;

class GOKNAR_API NavigationWorld
{
public:
	NavigationWorld();
	~NavigationWorld();

	void BuildFromScene(Scene* scene, const NavMeshSettings& settings);

	bool FindPath(const Vector3& start, const Vector3& end, std::vector<NavigationPath>& path);

protected:

private:
	NavigationMesh* navigationMesh_{ nullptr };
	PathFinder* pathFinder_{ nullptr };
};

#endif
