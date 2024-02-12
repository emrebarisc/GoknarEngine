#ifndef __OVERLAPPINGPHYSICSOBJECT_H__
#define __OVERLAPPINGPHYSICSOBJECT_H__

#include "Core.h"

#include "Goknar/Physics/PhysicsObject.h"
#include "Goknar/Physics/PhysicsTypes.h"

#include "LinearMath/btVector3.h"

class btRigidBody;

class GOKNAR_API OverlappingPhysicsObject : public PhysicsObject
{
public:
	OverlappingPhysicsObject();
	~OverlappingPhysicsObject();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

	virtual void PhysicsTick(float deltaTime) override;

	virtual void UpdateWorldTransformationMatrix() override;
protected:

private:
};

#endif