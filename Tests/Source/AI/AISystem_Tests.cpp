#include "AISystem_Tests.h"

#include "TestsCore.h"

#include "Goknar/AI/BehaviorTree.h"
#include "Goknar/AI/Blackboard.h"

void AISystem_Tests::Run()
{
	Blackboard blackboard;
	blackboard.SetValue<bool>("CanSeeTarget", true);
	blackboard.SetValue<int>("Ammo", 7);
	blackboard.SetValue<float>("Health", 85.5f);
	blackboard.SetValue<Vector3>("MoveTarget", Vector3(10.f, 20.f, 30.f));
	blackboard.SetStringValue("State", "Patrol");

	bool canSeeTarget = false;
	int ammo = 0;
	float health = 0.f;
	Vector3 moveTarget = Vector3::ZeroVector;

	TEST_ASSERT(blackboard.TryGetValue("CanSeeTarget", canSeeTarget) && canSeeTarget, "AI: blackboard bool read succeeds");
	TEST_ASSERT(blackboard.TryGetValue("Ammo", ammo) && ammo == 7, "AI: blackboard int read succeeds");
	TEST_ASSERT(blackboard.TryGetValue("Health", health) && health == 85.5f, "AI: blackboard float read succeeds");
	TEST_ASSERT(blackboard.TryGetValue("MoveTarget", moveTarget) && moveTarget == Vector3(10.f, 20.f, 30.f), "AI: blackboard vector read succeeds");
	TEST_ASSERT(blackboard.GetStringValue("State") && *blackboard.GetStringValue("State") == "Patrol", "AI: blackboard string read succeeds");
	TEST_ASSERT(blackboard.GetValueType("MoveTarget") == BlackboardValueType::Vector3, "AI: blackboard type query succeeds");
	TEST_ASSERT(!blackboard.TryGetValue<float>("Ammo", health), "AI: blackboard rejects mismatched type reads");
	TEST_ASSERT(blackboard.RemoveValue("State") && !blackboard.HasValue("State"), "AI: blackboard remove succeeds");

	AITickContext context;
	BehaviorTree tree;
	BehaviorSequenceNode* root = new BehaviorSequenceNode();
	root->AddChild(new BehaviorConditionNode("Has Target", [](AITickContext&) { return true; }));
	root->AddChild(new BehaviorTaskNode("Finish Task", [](AITickContext&) { return BehaviorStatus::Succeeded; }));
	tree.SetRootNode(root, true);

	TEST_ASSERT(tree.Tick(context) == BehaviorStatus::Succeeded, "AI: behavior sequence succeeds");
	TEST_ASSERT(tree.GetDebugData().size() == 3, "AI: behavior tree debug data contains all nodes");

	BehaviorTree selectorTree;
	BehaviorSelectorNode* selectorRoot = new BehaviorSelectorNode();
	selectorRoot->AddChild(new BehaviorConditionNode("Reject", [](AITickContext&) { return false; }));
	selectorRoot->AddChild(new BehaviorTaskNode("Fallback", [](AITickContext&) { return BehaviorStatus::Succeeded; }));
	selectorTree.SetRootNode(selectorRoot, true);

	TEST_ASSERT(selectorTree.Tick(context) == BehaviorStatus::Succeeded, "AI: behavior selector falls back to succeeding child");

	GOKNAR_CORE_INFO("All AISystem test sequences completed.");
}
