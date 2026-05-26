#ifndef __NAVIGATIONTREECOMPONENT_H__
#define __NAVIGATIONTREECOMPONENT_H__

#include <string>

#include "Goknar/Components/Component.h"
#include "Goknar/Navigation/NavigationTree.h"

class GOKNAR_API NavigationTreeComponent : public Component
{
public:
	NavigationTreeComponent() = delete;
	NavigationTreeComponent(const NavigationTreeComponent&) = delete;
	NavigationTreeComponent(Component* parent);
	~NavigationTreeComponent() override;

	Component* Clone() const override;

	bool SetNavigationTreePath(const std::string& navigationTreePath);
	const std::string& GetNavigationTreePath() const
	{
		return navigationTreePath_;
	}

	void SetRelativeNavigationTree(const NavigationTree& navigationTree);

	const NavigationTree& GetRelativeNavigationTree() const
	{
		return relativeNavigationTree_;
	}

	const NavigationTree& GetNavigationTree() const
	{
		return navigationTree_;
	}

	NavigationTree& GetNavigationTree()
	{
		return navigationTree_;
	}

	void UpdateNavigationTree();

protected:
	void UpdateComponentToWorldTransformationMatrix() override;

private:
	std::string navigationTreePath_;
	NavigationTree relativeNavigationTree_;
	NavigationTree navigationTree_;
};

#endif
