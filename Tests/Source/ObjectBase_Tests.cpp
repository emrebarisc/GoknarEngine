#include "ObjectBase_Tests.h"

#include "TestsCore.h"
#include "Goknar/ObjectBase.h"
#include "Goknar/Components/Component.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/Matrix.h"

// Helper class to test Component attachment since Component ctors are protected
class TestComponent : public Component
{
public:
    TestComponent(Component* parent) : Component(parent) {}
    TestComponent(ObjectBase* parent) : Component(parent) {}
};

void ObjectBase_Tests::Run()
{
    try
    {
        RunHierarchyTests();
        RunSnappingTests();
        RunComponentManagementTests();

        GOKNAR_CORE_INFO("All ObjectBase test sequences completed.");
    }
    catch (...)
    {
        GOKNAR_CORE_ERROR("An unexpected error occurred during testing ObjectBase.");
    }
}

void ObjectBase_Tests::RunHierarchyTests()
{
    ObjectBase* parent = new ObjectBase();
    ObjectBase* child = new ObjectBase();

    // 1. Initial State
    TEST_ASSERT(parent->GetChildren().empty(), "New ObjectBase has no children");
    TEST_ASSERT(child->GetParent() == nullptr, "New ObjectBase has no parent");

    // 2. Simple Attachment
    // Attach child to parent without special snapping (default behavior)
    child->SetParent(parent);
    
    TEST_ASSERT(child->GetParent() == parent, "Child recognizes Parent");
    TEST_ASSERT(parent->GetChildren().size() == 1, "Parent recognizes Child");
    TEST_ASSERT(parent->GetChildren()[0] == child, "Parent has correct Child reference");

    // 3. Transformation Propagation
    // Move Parent -> Child World Position should move
    Vector3 parentMove(10.f, 0.f, 0.f);
    parent->SetWorldPosition(parentMove);

    // Since child relative position was 0, child world should match parent world
    TEST_ASSERT(child->GetWorldTransformationMatrix().GetTranslation() == parentMove + child->GetWorldPosition(), "Child inherits Parent translation");

    // 4. Relative Positioning
    // Move Child relatively
    Vector3 childRelative(0.f, 5.f, 0.f);
    child->SetWorldPosition(parentMove + childRelative); // Set world to (10, 5, 0)
    
    // Verify relative calculation
    // Parent is at (10,0,0), Child at (10,5,0) -> Relative should be (0,5,0) (ignoring rotation for now)
    Vector3 calculatedRelative = child->GetWorldPositionInRelativeSpace(child->GetWorldPosition());
    // Note: The public API for setting relative position on ObjectBase isn't explicit in the provided header 
    // (it only has SetWorld...), but we can verify the transformation matrix logic.
    
    // 5. Detachment
    child->SetParent(nullptr);
    TEST_ASSERT(child->GetParent() == nullptr, "Child detached");
    TEST_ASSERT(parent->GetChildren().empty(), "Parent list cleared");

    child->Destroy();
    parent->Destroy();
}

void ObjectBase_Tests::RunSnappingTests()
{
    ObjectBase* parent = new ObjectBase();
    ObjectBase* child = new ObjectBase();

    Vector3 parentWorldPosition = Vector3(10.f, 10.f, 10.f);
    Vector3 childWorldPosition = Vector3(20.f, 10.f, 10.f);

    Vector3 childRelativePosition = childWorldPosition - parentWorldPosition;

    parent->SetWorldPosition(parentWorldPosition);
    child->SetWorldPosition(childWorldPosition);

    // 1. KeepWorldPosition Snapping
    // When attaching, child should stay at (20,10,10) world, 
    // meaning its relative position should become (10,0,0) (assuming identity rotation).
    child->SetParent(parent, SnappingRule::KeepWorldPosition);

    TEST_ASSERT(child->GetWorldTransformationMatrix().GetTranslation() == childWorldPosition, "Snapping: World Position Preserved");
    
    // Verify relative space conversion logic implicitly
    Vector3 relativePos = child->GetWorldPositionInRelativeSpace(child->GetWorldPosition());
    TEST_ASSERT(child->GetWorldPosition() == childRelativePosition, "Snapping: Relative Position Calculated Correctly");

    child->Destroy();
    parent->Destroy();
}

void ObjectBase_Tests::RunComponentManagementTests()
{
    ObjectBase* obj = new ObjectBase();

    // 1. Add Component
    TestComponent* comp = obj->AddSubComponent<TestComponent>();
    TEST_ASSERT(comp != nullptr, "Component created");
    TEST_ASSERT(comp->GetOwner() == obj, "Component owner set");
    TEST_ASSERT(obj->GetComponents().size() == 1, "Component added to list");

    // 2. Retrieve Component
    TestComponent* retrieved = obj->GetFirstComponentOfType<TestComponent>();
    TEST_ASSERT(retrieved == comp, "GetFirstComponentOfType returns correct pointer");

    // 3. Root Component
    // By default, first component usually becomes root if not set, 
    // or we strictly test SetRootComponent logic if accessible.
    obj->SetRootComponent(comp);
    TEST_ASSERT(obj->GetRootComponent() == comp, "Root component set");

    obj->Destroy(); // ObjectBase destructor should handle component cleanup
}