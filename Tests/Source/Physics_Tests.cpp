#include "Physics_Tests.h"

#include "TestsCore.h"
#include "Goknar/Engine.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/ObjectBase.h"
#include "Goknar/Physics/Components/BoxCollisionComponent.h"

#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "LinearMath/btTransform.h"

void Physics_Tests::Run()
{
	try
	{
		RunCollisionScaleRecoveryTests();

		GOKNAR_CORE_INFO("All Physics test sequences completed.");
	}
	catch (...)
	{
		GOKNAR_CORE_ERROR("An unexpected error occurred during testing Physics.");
	}
}

void Physics_Tests::RunCollisionScaleRecoveryTests()
{
	ObjectBase* owner = new ObjectBase();
	BoxCollisionComponent* boxCollisionComponent = owner->AddSubComponent<BoxCollisionComponent>();

	boxCollisionComponent->PreInit();
	boxCollisionComponent->Init();
	boxCollisionComponent->PostInit();

	owner->SetWorldScaling(Vector3::ZeroVector);
	const btVector3 zeroObjectScale = boxCollisionComponent->GetBulletCollisionShape()->getLocalScaling();
	TEST_ASSERT(
		zeroObjectScale.x() > 0.f &&
		zeroObjectScale.y() > 0.f &&
		zeroObjectScale.z() > 0.f,
		"Collision scaling uses a positive minimum instead of zero");

	owner->SetWorldScaling(Vector3(2.f, 3.f, 4.f));
	btTransform aabbTransform;
	aabbTransform.setIdentity();
	btVector3 aabbMin;
	btVector3 aabbMax;
	boxCollisionComponent->GetBulletCollisionShape()->getAabb(aabbTransform, aabbMin, aabbMax);

	TEST_ASSERT(!GoknarMath::IsNanOrInf(aabbMin.x()), "Collision AABB min X remains finite after zero scale recovery");
	TEST_ASSERT(!GoknarMath::IsNanOrInf(aabbMin.y()), "Collision AABB min Y remains finite after zero scale recovery");
	TEST_ASSERT(!GoknarMath::IsNanOrInf(aabbMin.z()), "Collision AABB min Z remains finite after zero scale recovery");
	TEST_ASSERT(!GoknarMath::IsNanOrInf(aabbMax.x()), "Collision AABB max X remains finite after zero scale recovery");
	TEST_ASSERT(!GoknarMath::IsNanOrInf(aabbMax.y()), "Collision AABB max Y remains finite after zero scale recovery");
	TEST_ASSERT(!GoknarMath::IsNanOrInf(aabbMax.z()), "Collision AABB max Z remains finite after zero scale recovery");

	const btVector3 recoveredScale = boxCollisionComponent->GetBulletCollisionShape()->getLocalScaling();
	TEST_ASSERT(GoknarMath::Abs(recoveredScale.x() - 2.f) < EPSILON, "Collision scaling recovers X after zero scale");
	TEST_ASSERT(GoknarMath::Abs(recoveredScale.y() - 3.f) < EPSILON, "Collision scaling recovers Y after zero scale");
	TEST_ASSERT(GoknarMath::Abs(recoveredScale.z() - 4.f) < EPSILON, "Collision scaling recovers Z after zero scale");

	owner->Destroy();
	engine->FlushPendingDestroy();
}
