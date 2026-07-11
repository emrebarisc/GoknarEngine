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
    TEST_ASSERT(child->GetWorldTransformationMatrix().GetTranslation().Equals(parentMove, EPSILON), "Child inherits Parent translation");
    TEST_ASSERT(child->GetWorldPosition().Equals(Vector3::ZeroVector, EPSILON), "Child keeps zero parent-relative translation");

    // 4. Relative Positioning
    // Move Child using its stored transform, which is parent-relative while parented.
    Vector3 childRelative(0.f, 5.f, 0.f);
    child->SetWorldPosition(childRelative);
    
    // Parent is at (10,0,0), Child world is at (10,5,0) -> Relative should be (0,5,0).
    TEST_ASSERT(child->GetWorldTransformationMatrix().GetTranslation().Equals(parentMove + childRelative, EPSILON), "Stored parent-relative position composes to actual world position");
    TEST_ASSERT(child->GetWorldPosition().Equals(childRelative, EPSILON), "SetWorldPosition stores parent-relative position");
    
    // 5. Detachment
    child->SetParent(nullptr);
    TEST_ASSERT(child->GetParent() == nullptr, "Child detached");
    TEST_ASSERT(parent->GetChildren().empty(), "Parent list cleared");
    TEST_ASSERT(child->GetWorldPosition().Equals(parentMove + childRelative, EPSILON), "Detaching preserves world position by default");

    child->Destroy();
    parent->Destroy();
}

void ObjectBase_Tests::RunSnappingTests()
{
    ObjectBase* parent = new ObjectBase();
    ObjectBase* child = new ObjectBase();

    Vector3 parentWorldPosition = Vector3(10.f, 10.f, 10.f);
    Vector3 childWorldPosition = Vector3(20.f, 10.f, 10.f);
    Quaternion parentWorldRotation = Quaternion::FromEulerDegrees(Vector3(0.f, 0.f, 90.f));
    Quaternion childWorldRotation = Quaternion::FromEulerDegrees(Vector3(0.f, 0.f, 180.f));
    Vector3 parentWorldScaling = Vector3(2.f, 4.f, 2.f);
    Vector3 childWorldScaling = Vector3(8.f, 12.f, 2.f);

    Vector3 childRelativePosition = childWorldPosition - parentWorldPosition;
    Vector3 childRelativeScaling = childWorldScaling / parentWorldScaling;

    parent->SetWorldPosition(parentWorldPosition);
    parent->SetWorldRotation(parentWorldRotation, false);
    parent->SetWorldScaling(parentWorldScaling);
    child->SetWorldPosition(childWorldPosition);
    child->SetWorldRotation(childWorldRotation, false);
    child->SetWorldScaling(childWorldScaling);

    // 1. KeepWorldAll Snapping
    // When attaching, child should keep its current world transform while storing values relative to parent.
    child->SetParent(parent, SnappingRule::KeepWorldAll);

    TEST_ASSERT(child->GetWorldTransformationMatrix().GetTranslation().Equals(childWorldPosition, EPSILON), "Snapping: World Position Preserved");
    TEST_ASSERT(Quaternion::GetAngleBetween(parentWorldRotation * child->GetWorldRotation(), childWorldRotation) < EPSILON, "Snapping: World Rotation Preserved");
    TEST_ASSERT((parentWorldScaling * child->GetWorldScaling()).Equals(childWorldScaling, EPSILON), "Snapping: World Scaling Preserved");
    TEST_ASSERT(child->GetWorldPosition().Equals(parent->GetWorldPositionInRelativeSpace(childWorldPosition), EPSILON), "Snapping: Relative Position Calculated Correctly");
    TEST_ASSERT(child->GetWorldScaling().Equals(childRelativeScaling, EPSILON), "Snapping: Relative Scaling Calculated Correctly");

    // 2. Relative transform writes should compose with the parent.
    child->SetWorldPosition(childRelativePosition);
    TEST_ASSERT(child->GetWorldTransformationMatrix().GetTranslation().Equals(parent->GetRelativePositionInWorldSpace(childRelativePosition), EPSILON), "Relative Position Composes With Parent");

    // 3. Detach should convert stored parent-relative values to actual world values.
    const Vector3 childActualWorldPosition = child->GetWorldTransformationMatrix().GetTranslation();
    child->SetParent(nullptr);
    TEST_ASSERT(child->GetWorldPosition().Equals(childActualWorldPosition, EPSILON), "Detaching Converts Stored Position To Actual World Space");

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
