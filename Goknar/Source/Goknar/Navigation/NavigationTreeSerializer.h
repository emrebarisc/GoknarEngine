#ifndef __NAVIGATION_TREE_SERIALIZER__
#define __NAVIGATION_TREE_SERIALIZER__

#include "Goknar/Core.h"

#include <string>

class NavigationTree;

class GOKNAR_API NavigationTreeSerializer
{
public:
	NavigationTreeSerializer() = default;
	~NavigationTreeSerializer() = default;

	static bool Serialize(const std::string& filepath, const NavigationTree& navigationTree);
	static bool Deserialize(const std::string& filepath, NavigationTree& outNavigationTree);
};

#endif
