#include "AICharacter.h"

#include "Goknar/AI/AIController.h"
#include "Goknar/AI/AITask.h"
#include "Goknar/AI/BehaviorTree.h"
#include "Goknar/AI/Blackboard.h"
#include "Goknar/AI/PerceptionComponent.h"
#include "Goknar/Navigation/NavAgentComponent.h"

#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Components/SocketComponent.h"
#include "Goknar/Physics/Components/CapsuleCollisionComponent.h"
#include "Goknar/Physics/Components/PhysicsMovementComponent.h"

#include "Goknar/Managers/ResourceManager.h"

#include "Goknar/Animation/AnimationGraph.h"
#include "Goknar/Animation/AnimationDeserializer.h"

#include "Goknar/Debug/DebugDrawer.h"

#include "Objects/Weapon.h"


void DrawPathResultDebug(const PathResult& pathResult)
{
	if (!pathResult.HasPath())
	{
		return;
	}

	const std::vector<NavPathPoint>& pathPoints =
		!pathResult.smoothedPath.empty()
		? pathResult.smoothedPath
		: pathResult.rawPath;

	if (pathPoints.empty())
	{
		return;
	}

	const Colorf nodeColor{ 1.f, 0.2f, 0.1f };
	const Colorf lineColor{ 0.1f, 1.f, 0.2f };

	const float nodeRadius = 0.125f;
	const float nodeThickness = 1.5f;
	const float lineThickness = 4.f;
	const float debugTime = -1.f;

	for (size_t i = 0; i < pathPoints.size(); ++i)
	{
		const Vector3& position = pathPoints[i].position;

		DebugDrawer::DrawSphere(
			position,
			Quaternion{},
			nodeRadius,
			nodeColor,
			nodeThickness,
			debugTime
		);

		if (i + 1 < pathPoints.size())
		{
			DebugDrawer::DrawLine(
				position,
				pathPoints[i + 1].position,
				lineColor,
				lineThickness,
				debugTime
			);
		}
	}
}

AICharacter::AICharacter() :
	BaseCharacter()
{
	SetIsTickable(true);

	SetName("AICharacter");
	SetTag("Character");

	capsuleCollisionComponent_->SetRadius(0.4f);
	capsuleCollisionComponent_->SetHeight(0.9f);
	capsuleCollisionComponent_->SetCollisionGroup(GetCollisionGroup());

	skeletalMesh_ = engine->GetResourceManager()->GetContent<SkeletalMesh>("Meshes/Characters/SK_DefaultCharacter.fbx");

	skeletalMeshComponent_->SetMesh(skeletalMesh_);
	skeletalMeshComponent_->SetParent(GetRootComponent());
	skeletalMeshComponent_->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3{ 90.f, 0.f, 90.f }));
	skeletalMeshComponent_->SetRelativeScaling(Vector3{ 0.01f });
	skeletalMeshComponent_->SetRelativePosition(Vector3{ 0.f, 0.f, -0.775f });

	SocketComponent* socketComponent = skeletalMeshComponent_->GetMeshInstance()->AddSocketToBone("mixamorig:RightHand");
	socketComponent->SetRelativePosition(Vector3{ 0.f, 12.5f, 5.f });
	socketComponent->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3{ 90.f, 0.f, 110.f }));
	socketComponent->SetRelativeScaling(Vector3{ 100.f });

	weapon_ = new Weapon();
	weapon_->AttachToSocket(socketComponent);

	animationGraph_ = new AnimationGraph();
	animationGraph_->relativeSkeletalMeshInstance = skeletalMeshComponent_->GetMeshInstance();
	AnimationDeserializer().Deserialize(animationGraph_, "Animations/AG_DefaultCharacter");

	PhysicsMovementComponent* defaultCharacterMovementComponent = GetMovementComponent();

	NavAgentComponent* navAgentComponent = AddSubComponent<NavAgentComponent>();
	navAgentComponent->SetMovementComponent(defaultCharacterMovementComponent);
	navAgentComponent->SetWaypointAcceptanceRadius(0.35f);
	navAgentComponent->SetLocalAvoidanceRadius(1.25f);
	NavAgentProfile agentProfile = navAgentComponent->GetAgentProfile();
	agentProfile.name = "AICharacter";
	agentProfile.radius = 0.4f;
	agentProfile.height = 1.8f;
	agentProfile.maxStepHeight = 0.35f;
	navAgentComponent->SetAgentProfile(agentProfile);

	PerceptionComponent* perceptionComponent = AddSubComponent<PerceptionComponent>();
	perceptionComponent->SetSightRadius(15.f);
	perceptionComponent->SetFieldOfViewDegrees(120.f);

	AIController* aiController = new AIController();
	aiController->SetControlledObject(this);
	aiController->SetNavigationAgentComponent(navAgentComponent);
	aiController->SetMovementComponent(defaultCharacterMovementComponent);
	aiController->GetBlackboard().SetValue<Vector3>("MoveTarget", moveTarget_);

	BehaviorTree* behaviorTree = new BehaviorTree();
	behaviorTree->SetRootNode(new BehaviorTaskNode(new AIMoveToTask("MoveTarget", 0.75f)), true);
	aiController->SetBehaviorTree(behaviorTree, true);

	controller_ = aiController;
}

AICharacter::AICharacter(const Vector3& spawnProbePosition, const Vector3& moveTarget) :
	AICharacter()
{
	spawnProbePosition_ = spawnProbePosition;
	moveTarget_ = moveTarget;
}

void AICharacter::BeginGame()
{
	Character::BeginGame();

	SetWorldPosition(Vector3{ -6.f, -2.f, 3.125f });
	SetMoveTarget(Vector3{ 6.f, -2.f, 0.125f });
}

void AICharacter::Tick(float)
{
	Vector3 linearVelocity = movementComponent_->GetLinearVelocity();
	Vector3 linearVelocity2D = Vector3(linearVelocity.x, linearVelocity.y, 0.f);
	float animation2DVelocityMagnitude = linearVelocity2D.Length();
	animationGraph_->SetVariable<float>("VelocityMagnitude", animation2DVelocityMagnitude);
	animationGraph_->SetVariable<float>("Speed", animation2DVelocityMagnitude);
	animationGraph_->SetVariable<bool>("IsOnGround", movementComponent_->OnGround());

	static bool zort = false;

	if (!zort)
	{
		AIController* aiController = dynamic_cast<AIController*>(controller_);
		if (aiController)
		{
			PathResult pathResult = aiController->GetNavigationAgentComponent()->GetCurrentPath();

			if (pathResult.HasPath())
			{
				DrawPathResultDebug(pathResult);
				zort = true;
			}
		}
	}
}

void AICharacter::SetSpawnProbePosition(const Vector3& spawnProbePosition)
{
	spawnProbePosition_ = spawnProbePosition;
}

void AICharacter::SetMoveTarget(const Vector3& moveTarget)
{
	moveTarget_ = moveTarget;

	AIController* aiController = dynamic_cast<AIController*>(controller_);
	if (aiController)
	{
		aiController->GetBlackboard().SetValue<Vector3>("MoveTarget", moveTarget_);
	}
}