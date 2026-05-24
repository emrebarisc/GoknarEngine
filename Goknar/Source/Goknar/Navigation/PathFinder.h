#ifndef __PATH_FINDER_H__
#define __PATH_FINDER_H__

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"

#include <vector>

class NavigationTree;
struct NavigationNode;

class GOKNAR_API NavigationPath
{
public:
	Vector3 position;
};

class GOKNAR_API PathFinder
{
public:
	PathFinder() = default;
	PathFinder(NavigationTree* navigationTree) : navigationTree_(navigationTree) {};
	~PathFinder() = default;

	bool FindPath(const Vector3& start, const Vector3& goal, std::vector<NavigationPath>& path);

private:
	void GetSmoothPath(const Vector3& start, const Vector3& goal, const std::vector<NavigationNode*>& nodePath, std::vector<NavigationPath>& resultPath);

	NavigationTree* navigationTree_{ nullptr };
};

#endif
