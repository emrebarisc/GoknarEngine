#include "pch.h"

#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "Engine.h"
#include "GoknarAssert.h"
#include "PhysicsDebugger.h"
#include "PhysicsUtils.h"
#include "PhysicsWorld.h"
#include "RigidBody.h"
#include "Components/PhysicsMovementComponent.h"
#include "Components/CollisionComponent.h"

#include <cstdint>

PhysicsWorld::PhysicsWorld()
{
	physicsDebugger_ = new PhysicsDebugger();
}

PhysicsWorld::~PhysicsWorld()
{
	delete dynamicsWorld_;
	dynamicsWorld_ = nullptr;

	delete solver_;
	solver_ = nullptr;

	delete broadphase_;
	broadphase_ = nullptr;

	delete ghostPairCallback_;
	ghostPairCallback_ = nullptr;

	delete dispatcher_;
	dispatcher_ = nullptr;

	delete collisionConfiguration_;
	collisionConfiguration_ = nullptr;

	delete overlappingCollisionPairCallback_;
	overlappingCollisionPairCallback_ = nullptr;

	delete physicsDebugger_;
	physicsDebugger_ = nullptr;
}

bool OverlappingDestroyedCallback(void* userPersistentData)
{
	return true;
}

bool OverlappingProcessedCallback(btManifoldPoint& manifoldPoint, void* body0, void* body1)
{
	engine->GetPhysicsWorld()->RecordOverlappingCollisionContinue(manifoldPoint, static_cast<const btCollisionObject*>(body0), static_cast<const btCollisionObject*>(body1));
	return true;
}

void OverlappingStartedCallback(btPersistentManifold* const& manifold)
{
	engine->GetPhysicsWorld()->RecordOverlappingCollisionBegin(manifold);
}

void OverlappingEndedCallback(btPersistentManifold* const& manifold)
{
	engine->GetPhysicsWorld()->RecordOverlappingCollisionEnd(manifold);
}

void PhysicsWorld::PreInit()
{
	///collision configuration contains default setup for memory, collision setup
	collisionConfiguration_ = new btDefaultCollisionConfiguration();
	//m_collisionConfiguration->setConvexConvexMultipointIterations();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher_ = new btCollisionDispatcher(collisionConfiguration_);
	//dispatcher_->setNearCallback(&NearCallback);

	ghostPairCallback_ = new btGhostPairCallback();

	broadphase_ = new btDbvtBroadphase();
	broadphase_->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback_);

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	solver_ = solver;

	dynamicsWorld_ = new btDiscreteDynamicsWorld(dispatcher_, broadphase_, solver_, collisionConfiguration_);
	dynamicsWorld_->setGravity(PhysicsUtils::FromVector3ToBtVector3(gravity_));
	dynamicsWorld_->setDebugDrawer(physicsDebugger_);

	//dynamicsWorld_->getDispatchInfo().m_allowedCcdPenetration = 0.04f;

	gContactDestroyedCallback = OverlappingDestroyedCallback;
	gContactProcessedCallback = OverlappingProcessedCallback;
	gContactStartedCallback = OverlappingStartedCallback;
	gContactEndedCallback = OverlappingEndedCallback;
}

void PhysicsWorld::Init()
{
}

void PhysicsWorld::PostInit()
{
}

void PhysicsWorld::PhysicsTick(float deltaTime)
{
	observedOverlaps_.clear();

	dynamicsWorld_->stepSimulation(deltaTime);

	GatherCurrentOverlaps();
	FlushOverlapEvents();

	for (PhysicsObject* physicsObject : physicsObjects_)
	{
		if (physicsObject->GetIsActive() && physicsObject->GetPhysicsTickEnabled())
		{
			physicsObject->PhysicsTick(deltaTime);
		}
	}

	for(PhysicsMovementComponent* physicsMovementComponent : physicsMovementComponents_)
	{
		physicsMovementComponent->UpdateOwnerTransformation();
	}

	DebugDraw();
}

void PhysicsWorld::SetPhysicsDebugMode(int debugMode)
{
	if (!physicsDebugger_)
	{
		return;
	}

	physicsDebugger_->setDebugMode(debugMode);
}

int PhysicsWorld::GetPhysicsDebugMode() const
{
	if (!physicsDebugger_)
	{
		return btIDebugDraw::DBG_NoDebug;
	}

	return physicsDebugger_->getDebugMode();
}

void PhysicsWorld::DebugDraw()
{
	if (!dynamicsWorld_ || !physicsDebugger_ || physicsDebugger_->getDebugMode() == btIDebugDraw::DBG_NoDebug)
	{
		return;
	}

	dynamicsWorld_->debugDrawWorld();
}

size_t PhysicsWorld::OverlapPairKeyHasher::operator()(const OverlapPairKey& pairKey) const
{
	const size_t object1Hash = std::hash<const btCollisionObject*>{}(pairKey.object1);
	const size_t object2Hash = std::hash<const btCollisionObject*>{}(pairKey.object2);
	return object1Hash ^ (object2Hash << 1);
}

PhysicsWorld::OverlapPairKey PhysicsWorld::CreateOverlapPairKey(const btCollisionObject* object1, const btCollisionObject* object2) const
{
	const std::uintptr_t object1Address = reinterpret_cast<std::uintptr_t>(object1);
	const std::uintptr_t object2Address = reinterpret_cast<std::uintptr_t>(object2);

	return object1Address < object2Address ? OverlapPairKey{ object1, object2 } : OverlapPairKey{ object2, object1 };
}

void PhysicsWorld::RecordOverlappingCollisionBegin(btPersistentManifold* const& manifold)
{
	RecordOverlap(manifold);
}

void PhysicsWorld::RecordOverlappingCollisionContinue(btManifoldPoint& monifoldPoint, const btCollisionObject* ghostObject1, const btCollisionObject* ghostObject2)
{
	RecordOverlap(ghostObject1, ghostObject2, monifoldPoint);
}

void PhysicsWorld::RecordOverlappingCollisionEnd(btPersistentManifold* const& manifold)
{
	(void)manifold;
}

void PhysicsWorld::RecordOverlap(const btPersistentManifold* manifold)
{
	if (!manifold || manifold->getNumContacts() == 0)
	{
		return;
	}

	const btManifoldPoint* bestManifoldPoint = nullptr;
	for (int contactIndex = 0; contactIndex < manifold->getNumContacts(); contactIndex++)
	{
		const btManifoldPoint& manifoldPoint = manifold->getContactPoint(contactIndex);
		if (manifold->getContactBreakingThreshold() < manifoldPoint.getDistance())
		{
			continue;
		}

		if (!bestManifoldPoint || manifoldPoint.getDistance() < bestManifoldPoint->getDistance())
		{
			bestManifoldPoint = &manifoldPoint;
		}
	}

	if (!bestManifoldPoint)
	{
		return;
	}

	RecordOverlap(manifold->getBody0(), manifold->getBody1(), *bestManifoldPoint);
}

void PhysicsWorld::RecordOverlap(const btCollisionObject* object1, const btCollisionObject* object2, const btManifoldPoint& manifoldPoint)
{
	RecordOverlap(
		object1,
		object2,
		PhysicsUtils::FromBtVector3ToVector3(manifoldPoint.getPositionWorldOnA()),
		PhysicsUtils::FromBtVector3ToVector3(manifoldPoint.getPositionWorldOnB()),
		PhysicsUtils::FromBtVector3ToVector3(manifoldPoint.m_normalWorldOnB),
		static_cast<float>(manifoldPoint.getDistance())
	);
}

void PhysicsWorld::RecordOverlap(const btCollisionObject* object1, const btCollisionObject* object2, const Vector3& worldPositionOnObject1, const Vector3& worldPositionOnObject2, const Vector3& hitNormal, float contactDistance)
{
	if (!object1 || !object2 || object1 == object2)
	{
		return;
	}

	const OverlapPairKey pairKey = CreateOverlapPairKey(object1, object2);
	OverlapState overlapState;
	overlapState.object1 = object1;
	overlapState.object2 = object2;
	overlapState.worldPositionOnObject1 = worldPositionOnObject1;
	overlapState.worldPositionOnObject2 = worldPositionOnObject2;
	overlapState.hitNormal = hitNormal;
	overlapState.contactDistance = contactDistance;

	auto observedOverlapIterator = observedOverlaps_.find(pairKey);
	if (observedOverlapIterator == observedOverlaps_.end())
	{
		observedOverlaps_.emplace(pairKey, overlapState);
		return;
	}

	if (contactDistance < observedOverlapIterator->second.contactDistance)
	{
		observedOverlapIterator->second = overlapState;
	}
}

void PhysicsWorld::GatherCurrentOverlaps()
{
	if (!dispatcher_)
	{
		return;
	}

	const int manifoldCount = dispatcher_->getNumManifolds();
	for (int manifoldIndex = 0; manifoldIndex < manifoldCount; manifoldIndex++)
	{
		RecordOverlap(dispatcher_->getManifoldByIndexInternal(manifoldIndex));
	}
}

void PhysicsWorld::FlushOverlapEvents()
{
	enum class OverlapEventType
	{
		Begin,
		Continue,
		End
	};

	struct OverlapEvent
	{
		OverlapEventType type;
		OverlapState state;
	};

	std::vector<OverlapEvent> overlapEvents;

	for (const auto& observedOverlap : observedOverlaps_)
	{
		const OverlapPairKey& pairKey = observedOverlap.first;
		const OverlapState& overlapState = observedOverlap.second;

		if (!ShouldEmitOverlap(overlapState))
		{
			activeOverlaps_.erase(pairKey);
			continue;
		}

		const bool wasActive = activeOverlaps_.find(pairKey) != activeOverlaps_.end();
		activeOverlaps_[pairKey] = overlapState;

		overlapEvents.push_back(OverlapEvent{ wasActive ? OverlapEventType::Continue : OverlapEventType::Begin, overlapState });
	}

	for (auto activeOverlapIterator = activeOverlaps_.begin(); activeOverlapIterator != activeOverlaps_.end();)
	{
		if (observedOverlaps_.find(activeOverlapIterator->first) != observedOverlaps_.end())
		{
			++activeOverlapIterator;
			continue;
		}

		const OverlapState endedOverlapState = activeOverlapIterator->second;
		activeOverlapIterator = activeOverlaps_.erase(activeOverlapIterator);

		if (ShouldEmitOverlap(endedOverlapState))
		{
			overlapEvents.push_back(OverlapEvent{ OverlapEventType::End, endedOverlapState });
		}
	}

	observedOverlaps_.clear();

	for (const OverlapEvent& overlapEvent : overlapEvents)
	{
		switch (overlapEvent.type)
		{
		case OverlapEventType::Begin:
			EmitOverlapBegin(overlapEvent.state);
			break;
		case OverlapEventType::Continue:
			EmitOverlapContinue(overlapEvent.state);
			break;
		case OverlapEventType::End:
			EmitOverlapEnd(overlapEvent.state);
			break;
		default:
			break;
		}
	}
}

void PhysicsWorld::RemoveOverlapStateForObject(const btCollisionObject* collisionObject)
{
	if (!collisionObject)
	{
		return;
	}

	for (auto activeOverlapIterator = activeOverlaps_.begin(); activeOverlapIterator != activeOverlaps_.end();)
	{
		const OverlapPairKey& pairKey = activeOverlapIterator->first;
		if (pairKey.object1 == collisionObject || pairKey.object2 == collisionObject)
		{
			activeOverlapIterator = activeOverlaps_.erase(activeOverlapIterator);
		}
		else
		{
			++activeOverlapIterator;
		}
	}

	for (auto observedOverlapIterator = observedOverlaps_.begin(); observedOverlapIterator != observedOverlaps_.end();)
	{
		const OverlapPairKey& pairKey = observedOverlapIterator->first;
		if (pairKey.object1 == collisionObject || pairKey.object2 == collisionObject)
		{
			observedOverlapIterator = observedOverlaps_.erase(observedOverlapIterator);
		}
		else
		{
			++observedOverlapIterator;
		}
	}
}

PhysicsObject* PhysicsWorld::GetPhysicsObjectFromCollisionObject(const btCollisionObject* collisionObject) const
{
	for (PhysicsObject* physicsObject : physicsObjects_)
	{
		if (physicsObject && physicsObject->GetBulletCollisionObject() == collisionObject)
		{
			return physicsObject;
		}
	}

	return nullptr;
}

bool PhysicsWorld::ShouldEmitOverlap(const OverlapState& overlapState) const
{
	PhysicsObject* collisionObject1 = GetPhysicsObjectFromCollisionObject(overlapState.object1);
	PhysicsObject* collisionObject2 = GetPhysicsObjectFromCollisionObject(overlapState.object2);

	if (!collisionObject1 || !collisionObject2)
	{
		return false;
	}

	CollisionComponent* collisionComponent1 = collisionObject1->GetCollisionComponent();
	CollisionComponent* collisionComponent2 = collisionObject2->GetCollisionComponent();

	return collisionComponent1 && collisionComponent2 && collisionComponent1->GetIsActive() && collisionComponent2->GetIsActive();
}

void PhysicsWorld::EmitOverlapBegin(const OverlapState& overlapState)
{
	PhysicsObject* collisionObject1 = GetPhysicsObjectFromCollisionObject(overlapState.object1);
	PhysicsObject* collisionObject2 = GetPhysicsObjectFromCollisionObject(overlapState.object2);

	if (!collisionObject1 || !collisionObject2)
	{
		return;
	}

	CollisionComponent* collisionComponent1 = collisionObject1->GetCollisionComponent();
	CollisionComponent* collisionComponent2 = collisionObject2->GetCollisionComponent();

	if (!collisionComponent1 || !collisionComponent2 || !collisionComponent1->GetIsActive() || !collisionComponent2->GetIsActive())
	{
		return;
	}

	collisionComponent1->OverlapBegin(collisionObject2, collisionComponent2, overlapState.worldPositionOnObject1, overlapState.hitNormal);
	collisionComponent2->OverlapBegin(collisionObject1, collisionComponent1, overlapState.worldPositionOnObject2, overlapState.hitNormal);
}

void PhysicsWorld::EmitOverlapContinue(const OverlapState& overlapState)
{
	PhysicsObject* collisionObject1 = GetPhysicsObjectFromCollisionObject(overlapState.object1);
	PhysicsObject* collisionObject2 = GetPhysicsObjectFromCollisionObject(overlapState.object2);

	if (!collisionObject1 || !collisionObject2)
	{
		return;
	}

	CollisionComponent* collisionComponent1 = collisionObject1->GetCollisionComponent();
	CollisionComponent* collisionComponent2 = collisionObject2->GetCollisionComponent();

	if (!collisionComponent1 || !collisionComponent2 || !collisionComponent1->GetIsActive() || !collisionComponent2->GetIsActive())
	{
		return;
	}

	collisionComponent1->OverlapContinue(collisionObject2, collisionComponent2, overlapState.worldPositionOnObject1, overlapState.hitNormal);
	collisionComponent2->OverlapContinue(collisionObject1, collisionComponent1, overlapState.worldPositionOnObject2, overlapState.hitNormal);
}

void PhysicsWorld::EmitOverlapEnd(const OverlapState& overlapState)
{
	PhysicsObject* collisionObject1 = GetPhysicsObjectFromCollisionObject(overlapState.object1);
	PhysicsObject* collisionObject2 = GetPhysicsObjectFromCollisionObject(overlapState.object2);

	if (!collisionObject1 || !collisionObject2)
	{
		return;
	}

	CollisionComponent* collisionComponent1 = collisionObject1->GetCollisionComponent();
	CollisionComponent* collisionComponent2 = collisionObject2->GetCollisionComponent();

	if (!collisionComponent1 || !collisionComponent2 || !collisionComponent1->GetIsActive() || !collisionComponent2->GetIsActive())
	{
		return;
	}

	collisionComponent1->OverlapEnd(collisionObject2, collisionComponent2);
	collisionComponent2->OverlapEnd(collisionObject1, collisionComponent1);
}
void PhysicsWorld::AddRigidBody(RigidBody* rigidBody)
{
	btRigidBody* bulletRigidBody = rigidBody->GetBulletRigidBody();

	dynamicsWorld_->addRigidBody(bulletRigidBody, (int)rigidBody->GetCollisionGroup(), (int)rigidBody->GetCollisionMask());
	physicsObjects_.push_back(rigidBody);
}

void PhysicsWorld::RemoveRigidBody(RigidBody* rigidBody)
{
	btRigidBody* bulletRigidBody = rigidBody->GetBulletRigidBody();

	if (!bulletRigidBody)
	{
		return;
	}

	decltype(physicsObjects_.begin()) physicsObjectIterator = physicsObjects_.begin();
	while(physicsObjectIterator != physicsObjects_.end())
	{
		if(*physicsObjectIterator == rigidBody)
		{
			physicsObjects_.erase(physicsObjectIterator);
			break;
		}

		++physicsObjectIterator;
	}

	dynamicsWorld_->removeRigidBody(bulletRigidBody);
	RemoveOverlapStateForObject(bulletRigidBody);
}

void PhysicsWorld::AddPhysicsObject(PhysicsObject* physicsObject)
{
	dynamicsWorld_->addCollisionObject(physicsObject->GetBulletCollisionObject(), (int)physicsObject->GetCollisionGroup(), (int)physicsObject->GetCollisionMask());
	physicsObjects_.push_back(physicsObject);
}

void PhysicsWorld::RemovePhysicsObject(PhysicsObject* physicsObject)
{
	btCollisionObject* bulletCollisionObject = physicsObject->GetBulletCollisionObject();

	if (!bulletCollisionObject)
	{
		return;
	}

	decltype(physicsObjects_.begin()) physicsObjectIterator = physicsObjects_.begin();
	while(physicsObjectIterator != physicsObjects_.end())
	{
		if(*physicsObjectIterator == physicsObject)
		{
			physicsObjects_.erase(physicsObjectIterator);
			break;
		}

		++physicsObjectIterator;
	}

	dynamicsWorld_->removeCollisionObject(bulletCollisionObject);
	RemoveOverlapStateForObject(bulletCollisionObject);
}

void PhysicsWorld::AddPhysicsMovementComponent(PhysicsMovementComponent* physicsMovementComponent)
{
	physicsMovementComponents_.push_back(physicsMovementComponent);
	dynamicsWorld_->addAction(physicsMovementComponent->GetBulletKinematicCharacterController());
}

void PhysicsWorld::RemovePhysicsMovementComponent(PhysicsMovementComponent* physicsMovementComponent)
{
	if (!physicsMovementComponent)
	{
		return;
	}

	decltype(physicsMovementComponents_.cbegin()) physicsMovementComponentIterator = physicsMovementComponents_.cbegin();
	while(physicsMovementComponentIterator != physicsMovementComponents_.cend())
	{
		if(*physicsMovementComponentIterator == physicsMovementComponent)
		{
			physicsMovementComponents_.erase(physicsMovementComponentIterator);
			break;
		}

		++physicsMovementComponentIterator;
	}

	dynamicsWorld_->removeAction(physicsMovementComponent->GetBulletKinematicCharacterController());
}

bool PhysicsWorld::RaycastClosest(const RaycastData& raycastData, RaycastSingleResult& raycastClosest)
{
	btVector3 bulletFrom = PhysicsUtils::FromVector3ToBtVector3(raycastData.from);
	btVector3 bulletTo = PhysicsUtils::FromVector3ToBtVector3(raycastData.to);

	btCollisionWorld::ClosestRayResultCallback closestRayResultCallback(bulletFrom, bulletTo);

	closestRayResultCallback.m_collisionFilterGroup = (int)raycastData.collisionGroup;
	closestRayResultCallback.m_collisionFilterMask = (int)raycastData.collisionMask;

	dynamicsWorld_->rayTest(bulletFrom, bulletTo, closestRayResultCallback);

	if(closestRayResultCallback.hasHit())
	{
		raycastClosest.hitObject = (PhysicsObject*)closestRayResultCallback.m_collisionObject->getUserPointer();
		raycastClosest.hitFraction = closestRayResultCallback.m_closestHitFraction;
		raycastClosest.hitPosition = PhysicsUtils::FromBtVector3ToVector3(closestRayResultCallback.m_hitPointWorld);
		raycastClosest.hitNormal = PhysicsUtils::FromBtVector3ToVector3(closestRayResultCallback.m_hitNormalWorld);

		return true;
	}

	return false;
}

bool PhysicsWorld::RaycastAll(const RaycastData& raycastData, RaycastAllResult& raycastAllResult)
{
	btVector3 bulletFrom = PhysicsUtils::FromVector3ToBtVector3(raycastData.from);
	btVector3 bulletTo = PhysicsUtils::FromVector3ToBtVector3(raycastData.to);

	btCollisionWorld::AllHitsRayResultCallback allHitsRayResultCallback(bulletFrom, bulletTo);

	allHitsRayResultCallback.m_collisionFilterGroup = (int)raycastData.collisionGroup;
	allHitsRayResultCallback.m_collisionFilterMask = (int)raycastData.collisionMask;

	dynamicsWorld_->rayTest(bulletFrom, bulletTo, allHitsRayResultCallback);

	if(allHitsRayResultCallback.hasHit())
	{
		const int hitCount = allHitsRayResultCallback.m_collisionObjects.size();
		for (int hitIndex = 0; hitIndex < hitCount; hitIndex++)
		{
			raycastAllResult.hitResults.emplace_back(
				RaycastSingleResult(
					(PhysicsObject*)allHitsRayResultCallback.m_collisionObjects[hitIndex]->getUserPointer(),
					PhysicsUtils::FromBtVector3ToVector3(allHitsRayResultCallback.m_hitPointWorld[hitIndex]),
					PhysicsUtils::FromBtVector3ToVector3(allHitsRayResultCallback.m_hitNormalWorld[hitIndex]),
					allHitsRayResultCallback.m_hitFractions[hitIndex]
				)
			);
		}

		return true;
	}

	return false;
}

bool PhysicsWorld::SweepClosest(const SweepData& sweepData, RaycastSingleResult& result)
{
	GOKNAR_CORE_ASSERT(sweepData.collisionComponent->GetBulletCollisionShape()->isConvex());

	btConvexShape* bulletcollisionShape = (btConvexShape*)sweepData.collisionComponent->GetBulletCollisionShape();
	
	btTransform bulletFromTransform = PhysicsUtils::GetBulletTransform(sweepData.fromRotation, sweepData.fromPosition);
	btTransform bulletToTransform = PhysicsUtils::GetBulletTransform(sweepData.toRotation, sweepData.toPosition);

	btCollisionWorld::ClosestConvexResultCallback closestResultCallback(
		PhysicsUtils::FromVector3ToBtVector3(sweepData.fromPosition),
		PhysicsUtils::FromVector3ToBtVector3(sweepData.toPosition)
	);

	closestResultCallback.m_collisionFilterGroup = (int)sweepData.collisionGroup;
	closestResultCallback.m_collisionFilterMask = (int)sweepData.collisionMask;

	dynamicsWorld_->convexSweepTest(bulletcollisionShape, bulletFromTransform, bulletToTransform, closestResultCallback, sweepData.ccdPenetration);

	if(closestResultCallback.hasHit())
	{
		result.hitObject = (PhysicsObject*)closestResultCallback.m_hitCollisionObject->getUserPointer();
		result.hitFraction = closestResultCallback.m_closestHitFraction;
		result.hitPosition = PhysicsUtils::FromBtVector3ToVector3(closestResultCallback.m_hitPointWorld);
		result.hitNormal = PhysicsUtils::FromBtVector3ToVector3(closestResultCallback.m_hitNormalWorld);

		return true;
	}
	
	return false;
}
