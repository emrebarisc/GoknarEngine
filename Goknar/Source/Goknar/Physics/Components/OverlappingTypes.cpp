#include "pch.h"
#include "OverlappingTypes.h"

btBroadphasePair* OverlappingCollisionPairCallback::addOverlappingPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1)
{
	btCollisionObject* colObj0 = (btCollisionObject*)proxy0->m_clientObject;
	btCollisionObject* colObj1 = (btCollisionObject*)proxy1->m_clientObject;
	btGhostObject* ghost0 = btGhostObject::upcast(colObj0);
	btGhostObject* ghost1 = btGhostObject::upcast(colObj1);

	if(!OnAddOverlappingPair.isNull())
	{
		OnAddOverlappingPair(colObj0, colObj1);
	}

    return btGhostPairCallback::addOverlappingPair(proxy0, proxy1);
}

void* OverlappingCollisionPairCallback::removeOverlappingPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1, btDispatcher* dispatcher)
{
	btCollisionObject* colObj0 = (btCollisionObject*)proxy0->m_clientObject;
	btCollisionObject* colObj1 = (btCollisionObject*)proxy1->m_clientObject;
	btGhostObject* ghost0 = btGhostObject::upcast(colObj0);
	btGhostObject* ghost1 = btGhostObject::upcast(colObj1);

	if(!OnRemoveOverlappingPair.isNull())
	{
		OnRemoveOverlappingPair(colObj0, colObj1);
	}
    
    return btGhostPairCallback::removeOverlappingPair(proxy0, proxy1, dispatcher);
}	