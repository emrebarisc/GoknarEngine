#ifndef __SPHERECOLLISIONCOMPONENT_H__
#define __SPHERECOLLISIONCOMPONENT_H__

#include "CollisionComponent.h"

class GOKNAR_API SphereCollisionComponent : public CollisionComponent
{
public:
	SphereCollisionComponent(Component* parent);
	SphereCollisionComponent(ObjectBase* parentObjectBase);
	~SphereCollisionComponent();

protected:
private:
};

#endif