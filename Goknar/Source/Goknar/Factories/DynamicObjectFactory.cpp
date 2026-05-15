#include "pch.h"

#include "DynamicObjectFactory.h"

#include "Goknar/AI/AIController.h"
#include "Goknar/AI/PerceptionComponent.h"
#include "Goknar/Components/CameraComponent.h"
#include "Goknar/Components/DynamicMeshComponent.h"
#include "Goknar/Components/InstancedStaticMeshComponent.h"
#include "Goknar/Components/LightComponents/PointLightComponent.h"
#include "Goknar/Components/ParticleSystemComponent.h"
#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Components/SocketComponent.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Debug/DebugDrawer.h"
#include "Goknar/Navigation/NavAgentComponent.h"
#include "Goknar/ObjectBase.h"
#include "Goknar/Objects/ReflectionProbeObject.h"
#include "Goknar/Physics/Character.h"
#include "Goknar/Physics/OverlappingPhysicsObject.h"
#include "Goknar/Physics/PhysicsObject.h"
#include "Goknar/Physics/RigidBody.h"
#include "Goknar/Physics/Components/BoxCollisionComponent.h"
#include "Goknar/Physics/Components/CapsuleCollisionComponent.h"
#include "Goknar/Physics/Components/MovingTriangleMeshCollisionComponent.h"
#include "Goknar/Physics/Components/MultipleCollisionComponent.h"
#include "Goknar/Physics/Components/NonMovingTriangleMeshCollisionComponent.h"
#include "Goknar/Physics/Components/PhysicsMovementComponent.h"
#include "Goknar/Physics/Components/SphereCollisionComponent.h"
#include "Goknar/UI/HUD.h"

DynamicObjectFactory* DynamicObjectFactory::instance_ = nullptr;

void DynamicObjectFactory::RegisterBuiltInClasses()
{
	static bool areBuiltInClassesRegistered = false;
	if (areBuiltInClassesRegistered || instance_ == nullptr)
	{
		return;
	}

	instance_->RegisterObjectClass<ObjectBase>("ObjectBase");
	instance_->RegisterObjectClass<DebugObject>("DebugObject");
	instance_->RegisterObjectClass<HUD>("HUD");
	instance_->RegisterObjectClass<PhysicsObject>("PhysicsObject");
	instance_->RegisterObjectClass<OverlappingPhysicsObject>("OverlappingPhysicsObject");
	instance_->RegisterObjectClass<RigidBody>("RigidBody");
	instance_->RegisterObjectClass<Character>("Character");
	instance_->RegisterObjectClass<AIController>("AIController");
	instance_->RegisterObjectClass<ReflectionProbeObject>("ReflectionProbeObject");

	instance_->RegisterComponentClass<SocketComponent>("SocketComponent", DynamicComponentOwnerRequirement::ObjectBase, false);
	instance_->RegisterComponentClass<CameraComponent>("CameraComponent");
	instance_->RegisterComponentClass<PointLightComponent>("PointLightComponent");
	instance_->RegisterComponentClass<DynamicMeshComponent>("DynamicMeshComponent", DynamicComponentOwnerRequirement::ObjectBase, false);
	instance_->RegisterComponentClass<InstancedStaticMeshComponent>("InstancedStaticMeshComponent");
	instance_->RegisterComponentClass<StaticMeshComponent>("StaticMeshComponent");
	instance_->RegisterComponentClass<SkeletalMeshComponent>("SkeletalMeshComponent");
	instance_->RegisterComponentClass<BillboardParticleSystemComponent>("BillboardParticleSystemComponent");
	instance_->RegisterComponentClass<StaticMeshParticleSystemComponent>("StaticMeshParticleSystemComponent");
	instance_->RegisterComponentClass<NavAgentComponent>("NavAgentComponent");
	instance_->RegisterComponentClass<PerceptionComponent>("PerceptionComponent");
	instance_->RegisterComponentClass<BoxCollisionComponent>("BoxCollisionComponent", DynamicComponentOwnerRequirement::PhysicsObject);
	instance_->RegisterComponentClass<CapsuleCollisionComponent>("CapsuleCollisionComponent", DynamicComponentOwnerRequirement::PhysicsObject);
	instance_->RegisterComponentClass<SphereCollisionComponent>("SphereCollisionComponent", DynamicComponentOwnerRequirement::PhysicsObject);
	instance_->RegisterComponentClass<MovingTriangleMeshCollisionComponent>("MovingTriangleMeshCollisionComponent", DynamicComponentOwnerRequirement::PhysicsObject);
	instance_->RegisterComponentClass<NonMovingTriangleMeshCollisionComponent>("NonMovingTriangleMeshCollisionComponent", DynamicComponentOwnerRequirement::PhysicsObject);
	instance_->RegisterComponentClass<MultipleCollisionComponent>("MultipleCollisionComponent", DynamicComponentOwnerRequirement::PhysicsObject, false);
	instance_->RegisterComponentClass<PhysicsMovementComponent>("PhysicsMovementComponent", DynamicComponentOwnerRequirement::OverlappingPhysicsObject);

	areBuiltInClassesRegistered = true;
}
