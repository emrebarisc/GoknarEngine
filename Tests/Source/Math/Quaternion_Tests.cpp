#include "Math/Quaternion_Tests.h"

#include "TestsCore.h"

#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/Quaternion.h"

void Quaternion_Tests::Run()
{
    try 
    {
        RunQuaternionTests();

        GOKNAR_CORE_INFO("All test sequences completed.");
    }
    catch (...)
    {

        GOKNAR_CORE_ERROR("An unexpected error occurred during testing.");
    }
}

void Quaternion_Tests::RunQuaternionTests()
{
    // 1. Initialization and Identity
    Quaternion qId;
    TEST_ASSERT(qId.x == 0.f && qId.y == 0.f && qId.z == 0.f && qId.w == 1.f, "Quaternion Default Constructor");

    // 2. Conjugate
    Quaternion q1(1.f, 2.f, 3.f, 4.f);
    Quaternion qConj = q1.GetConjugate();
    TEST_ASSERT(qConj.x == -1.f && qConj.y == -2.f && qConj.z == -3.f && qConj.w == 4.f, "Quaternion Conjugate");

    // 3. Length and Normalization
    Quaternion qNorm(0.f, 3.f, 4.f, 0.f); // Length should be 5
    TEST_ASSERT(GoknarMath::Abs(qNorm.Length() - 5.0f) < EPSILON, "Quaternion Length");
    
    qNorm.Normalize();
    TEST_ASSERT(GoknarMath::Abs(qNorm.Length() - 1.0f) < EPSILON, "Quaternion Normalization");
    TEST_ASSERT(GoknarMath::Abs(qNorm.y - 0.6f) < EPSILON, "Quaternion Normalized Value");

    // 4. Arithmetic Operators
    Quaternion a(1, 1, 1, 1);
    Quaternion b(2, 2, 2, 2);
    Quaternion c = a + b;
    TEST_ASSERT(c.x == 3.f && c.w == 3.f, "Quaternion Addition");

    // 5. Quaternion Multiplication (Rotation Concatenation)
    // Rotating 90 degrees around Z axis: x=0, y=0, z=sin(45), w=cos(45)
    float s = sinf(PI / 4.f);
    float r = cosf(PI / 4.f);
    Quaternion qRotZ90(0, 0, s, r);
    
    // Multiplying Identity by rotation should equal rotation
    Quaternion qResult = Quaternion::Identity * qRotZ90;
    TEST_ASSERT(GoknarMath::Abs(qResult.z - s) < EPSILON, "Quaternion Multiplication (Identity)");

    // 6. Vector Rotation
    // Rotate Vector (1, 0, 0) 90 degrees around Z axis -> should become (0, 1, 0)
    Vector3 v(1.0f, 0.0f, 0.0f);
    Vector3 rotatedV = qRotZ90.Rotate(v);
    TEST_ASSERT(GoknarMath::Abs(rotatedV.x) < EPSILON && 
                GoknarMath::Abs(rotatedV.y - 1.0f) < EPSILON, "Quaternion Rotate Vector");

    // 7. Inverse
    Quaternion qInv = qRotZ90.GetInverse();
    Quaternion qCheck = qRotZ90 * qInv;
    TEST_ASSERT(GoknarMath::Abs(qCheck.w - 1.0f) < EPSILON && 
                GoknarMath::Abs(qCheck.x) < EPSILON, "Quaternion Inverse (q * q^-1 = I)");

    // 8. Euler Conversion (Simple check)
    Quaternion qEuler = Quaternion::FromEulerDegrees(0, 0, 90.0f);
    TEST_ASSERT(GoknarMath::Abs(qEuler.z - s) < EPSILON, "Quaternion FromEulerDegrees");
}