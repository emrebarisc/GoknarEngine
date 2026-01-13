#include "Component_Tests.h"

#include "TestsCore.h"
#include "Goknar/Components/Component.h"
#include "Goknar/ObjectBase.h"
#include "Goknar/Math/Matrix.h"
#include "Goknar/Math/GoknarMath.h"

// Helper class to expose protected Component constructors
class TestComponent : public Component
{
public:
    TestComponent(Component* parent) : Component(parent) {}
    TestComponent(ObjectBase* parent) : Component(parent) {}
};

void Component_Tests::Run()
{
    try 
    {
        RunTransformTests();
        RunPivotTests();
        RunHierarchyTests();

        GOKNAR_CORE_INFO("All Component test sequences completed.");
    }
    catch (...)
    {
        GOKNAR_CORE_ERROR("An unexpected error occurred during testing Component.");
    }
}

void Component_Tests::RunTransformTests()
{
    ObjectBase* owner = new ObjectBase();
    // Owner at origin
    TestComponent* comp = new TestComponent(owner);

    // 1. Relative Position -> World Position
    Vector3 relPos(5.f, 0.f, 0.f);
    comp->SetRelativePosition(relPos);
    
    // Since owner is at (0,0,0) identity, World should equal Relative
    TEST_ASSERT(comp->GetWorldPosition() == relPos, "Relative Position propagates to World (Identity Parent)");
    TEST_ASSERT(comp->GetRelativePosition() == relPos, "GetRelativePosition returns set value");

    // 2. Relative Scaling
    Vector3 scale(2.f, 2.f, 2.f);
    comp->SetRelativeScaling(scale);
    TEST_ASSERT(comp->GetRelativeScaling() == scale, "Scaling set correctly");

    // 3. Forward Vector Calculation
    // Rotate 90 degrees around Z axis. Forward (1,0,0) should become (0,1,0) approximately.
    Quaternion rot = Quaternion::FromEulerDegrees(Vector3(0.f, 0.f, 90.f));
    comp->SetRelativeRotation(rot);

    Vector3 forward = comp->GetRelativeForwardVector();
    // Depending on coordinate system, Forward might be Y+ or Y- after Z rotation.
    // Assuming Standard: X=Forward. Z-Rot 90 -> Y=Forward.
    TEST_ASSERT(GoknarMath::Abs(forward.x) < EPSILON, "Forward X is 0 after 90 Z rot");
    TEST_ASSERT(GoknarMath::Abs(forward.y - 1.f) < EPSILON, "Forward Y is 1 after 90 Z rot");

    owner->Destroy(); // Deletes components
}

void Component_Tests::RunPivotTests()
{
    ObjectBase* owner = new ObjectBase();
    TestComponent* comp = new TestComponent(owner);

    // 1. Set Pivot
    Vector3 pivot(1.f, 0.f, 0.f);
    comp->SetPivotPoint(pivot);
    TEST_ASSERT(comp->GetPivotPoint() == pivot, "Pivot point set");

    // 2. Rotation around Pivot
    // If we rotate 180 degrees around Z, and pivot is at (1,0,0).
    // The relative position is (0,0,0).
    // Matrix logic: Translate(Pivot) * Rotate * Translate(-Pivot).
    // This effectively orbits the object around the pivot point inversely.
    
    // Testing strict matrix generation:
    comp->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3(0.f, 0.f, 180.f)));
    
    // We check the Relative Transformation Matrix directly
    const Matrix& mat = comp->GetRelativeTransformationMatrix();
    
    // A rotation of 180 around Z flips X and Y.
    // With pivot offset, this usually results in a translation component in the final matrix.
    // (1,0,0) -> rotate 180 -> (-1,0,0). 
    // Shift logic: (0,0,0) is the origin. Pivot is 1 unit away.
    // If the pivot logic is "rotate mesh around pivot", the visual mesh moves.
    // If the pivot logic is "transform anchor", the matrix translation changes.
    // Based on `Component.h`: 
    // Matrix = T(Pivot) * T(RelPos) * R(RelRot) * S(RelScale) * T(-Pivot)
    
    // We check that the matrix is NOT Identity despite Position being 0 and Scale being 1
    // because the rotation combined with pivot introduces translation.
    TEST_ASSERT(!(mat == Matrix::IdentityMatrix), "Pivot + Rotation affects Matrix");

    owner->Destroy();
}

void Component_Tests::RunHierarchyTests()
{
    ObjectBase* owner = new ObjectBase();
    
    // Parent Component
    TestComponent* parentComp = new TestComponent(owner);
    parentComp->SetRelativePosition(Vector3(10.f, 0.f, 0.f));

    // Child Component
    TestComponent* childComp = new TestComponent(parentComp);
    childComp->SetRelativePosition(Vector3(5.f, 0.f, 0.f));

    // 1. Parent-Child Relationship
    TEST_ASSERT(childComp->GetParent() == parentComp, "Child component has correct parent");
    TEST_ASSERT(parentComp->GetChildren().size() == 1, "Parent component has child");

    // 2. World Position Calculation
    // Parent World: (10, 0, 0)
    // Child Relative: (5, 0, 0)
    // Child World should be (15, 0, 0)
    TEST_ASSERT(childComp->GetWorldPosition().Equals(Vector3(15.f, 0.f, 0.f), EPSILON), "Child World Position composed correctly");

    // 3. Move Parent
    parentComp->SetRelativePosition(Vector3(20.f, 0.f, 0.f));
    // Child World should now be (25, 0, 0)
    TEST_ASSERT(childComp->GetWorldPosition().Equals(Vector3(25.f, 0.f, 0.f), EPSILON), "Child tracks Parent movement");

    owner->Destroy();
}