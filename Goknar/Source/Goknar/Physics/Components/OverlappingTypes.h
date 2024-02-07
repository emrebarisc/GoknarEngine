#ifndef __OVERLAPPINGTYPES_H__
#define __OVERLAPPINGTYPES_H__

#include "Core.h"
#include "Delegates/Delegate.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"

class GOKNAR_API OverlappingCollisionPairCallback : public btGhostPairCallback
{
public:
    OverlappingCollisionPairCallback() : btGhostPairCallback() {}

	virtual btBroadphasePair* addOverlappingPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) override;
	virtual void* removeOverlappingPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1, btDispatcher* dispatcher) override;

    Delegate<void(btCollisionObject*, btCollisionObject*)> OnAddOverlappingPair;
    Delegate<void(btCollisionObject*, btCollisionObject*)> OnRemoveOverlappingPair;
};

#endif
