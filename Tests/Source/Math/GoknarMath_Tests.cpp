#include "Math/GoknarMath_Tests.h"

#include "TestsCore.h"

#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/Matrix.h"
#include "Goknar/Math/MathDefines.h"

void GoknarMath_Tests::Run()
{
    try 
    {
        RunMacroTests();
        RunMathUtilsTests();
        RunVector2Tests();
        RunVector3Tests();
        RunVector4Tests();
        RunStaticMathTests();

        GOKNAR_CORE_INFO("All GoknarMath test sequences completed.");
    }
    catch (...)
    {

        GOKNAR_CORE_ERROR("An unexpected error occurred during testing GoknarMath.");
    }
}

void GoknarMath_Tests::RunMacroTests()
{
    TEST_ASSERT(mathMax(5, 10) == 10, "GoknarMath_Tests: mathMax(5, 10)");
    TEST_ASSERT(mathMin(5, 10) == 5, "GoknarMath_Tests: mathMin(5, 10)");
    TEST_ASSERT(mathClamp(15, 0, 10) == 10, "GoknarMath_Tests: mathClamp upper bound");
    TEST_ASSERT(mathClamp(-5, 0, 10) == 0, "GoknarMath_Tests: mathClamp lower bound");
    TEST_ASSERT(mathAbs(-5.5f) == 5.5f, "GoknarMath_Tests: mathAbs negative");
}

void GoknarMath_Tests::RunMathUtilsTests()
{
    TEST_ASSERT(GoknarMath::Lerp(0.0f, 10.0f, 0.5f) == 5.0f, "GoknarMath_Tests: Lerp 50%");
    
    TEST_ASSERT(GoknarMath::IsNanOrInf(std::nanf("")), "GoknarMath_Tests: IsNanOrInf NaN check");
    TEST_ASSERT(!GoknarMath::IsNanOrInf(5.0f), "GoknarMath_Tests: IsNanOrInf valid number check");

    TEST_ASSERT(GoknarMath::Min(10, 20) == 10, "GoknarMath_Tests: GoknarMath::Min");
    TEST_ASSERT(GoknarMath::Max(10, 20) == 20, "GoknarMath_Tests: GoknarMath::Max");
    TEST_ASSERT(GoknarMath::Floor(2.9f) == 2.0f, "GoknarMath_Tests: GoknarMath::Floor");
    TEST_ASSERT(GoknarMath::FloorToInt(2.9f) == 2, "GoknarMath_Tests: GoknarMath::FloorToInt");
    
    float frac = GoknarMath::Frac(1.75f);
    TEST_ASSERT(frac > 0.74f && frac < 0.76f, "GoknarMath_Tests: GoknarMath::Frac (1.75 -> 0.75)");
}

void GoknarMath_Tests::RunVector2Tests()
{
    Vector2 v1(3.0f, 4.0f);
    Vector2 v2(1.0f, 2.0f);

    TEST_ASSERT(v1.Length() == 5.0f, "GoknarMath_Tests: Vector2 Length (3,4) = 5");
    
    Vector2 vSum = v1 + v2;
    TEST_ASSERT(vSum.x == 4.0f && vSum.y == 6.0f, "GoknarMath_Tests: Vector2 Addition");

    float dot = Vector2::Dot(v1, v2);
    TEST_ASSERT(dot == 11.0f, "GoknarMath_Tests: Vector2 Dot Product");

    v1.Clamp(0.0f, 3.5f);
    TEST_ASSERT(v1.y == 3.5f, "GoknarMath_Tests: Vector2 Clamp");
}

void GoknarMath_Tests::RunVector3Tests()
{
    Vector3 v1(1.0f, 0.0f, 0.0f);
    Vector3 v2(0.0f, 1.0f, 0.0f);

    Vector3 cross = Vector3::Cross(v1, v2);
    TEST_ASSERT(cross.x == 0.0f && cross.y == 0.0f && cross.z == 1.0f, "GoknarMath_Tests: Vector3 Cross Product");

    Vector3 v3(5.0f, 0.0f, 0.0f);
    Vector3 norm = v3.GetNormalized();
    TEST_ASSERT(norm.x == 1.0f && norm.Length() == 1.0f, "GoknarMath_Tests: Vector3 Normalize");

    float dist = Vector3::Distance(Vector3(0,0,0), Vector3(0,10,0));
    TEST_ASSERT(dist == 10.0f, "GoknarMath_Tests: Vector3 Distance");

    Vector3 v4 = v1 * 5.0f;
    TEST_ASSERT(v4.x == 5.0f, "GoknarMath_Tests: Vector3 Scalar Multiplication");
}

void GoknarMath_Tests::RunVector4Tests()
{
    Vector4 v1(1, 2, 3, 4);
    Vector4 v2(1, 1, 1, 1);
    
    Vector4 res = v1 - v2;
    TEST_ASSERT(res.x == 0 && res.w == 3, "GoknarMath_Tests: Vector4 Subtraction");
    
    TEST_ASSERT(v1.ToString().find("1.0") != std::string::npos, "GoknarMath_Tests: Vector4 ToString check");
}

void GoknarMath_Tests::RunStaticMathTests()
{
    // 1. Degree/Radian Conversion
    float rad = DEGREE_TO_RADIAN(180.f);
    TEST_ASSERT(GoknarMath::Abs(rad - PI) < EPSILON, "DEGREE_TO_RADIAN");

    float deg = RADIAN_TO_DEGREE(PI / 2.f);
    TEST_ASSERT(GoknarMath::Abs(deg - 90.f) < EPSILON, "RADIAN_TO_DEGREE");

    // 2. Determinant Calculations
    // 2x2 Determinant
    // | 1 2 |
    // | 3 4 | -> 1*4 - 2*3 = -2
    float det2 = GoknarMath::Determinant(Vector2(1.f, 2.f), Vector2(3.f, 4.f));
    TEST_ASSERT(GoknarMath::Abs(det2 - (-2.f)) < EPSILON, "Determinant 2x2");

    // 3x3 Determinant (Identity should be 1)
    float det3 = GoknarMath::Determinant(Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1));
    TEST_ASSERT(GoknarMath::Abs(det3 - 1.f) < EPSILON, "Determinant 3x3 Identity");

    // 3. Min/Max/Clamp
    TEST_ASSERT(GoknarMath::Min(10.f, 20.f) == 10.f, "GoknarMath::Min");
    TEST_ASSERT(GoknarMath::Max(10.f, 20.f) == 20.f, "GoknarMath::Max");
    TEST_ASSERT(GoknarMath::Clamp(15.f, 0.f, 10.f) == 10.f, "GoknarMath::Clamp Upper");
    TEST_ASSERT(GoknarMath::Clamp(-5.f, 0.f, 10.f) == 0.f, "GoknarMath::Clamp Lower");

    // 4. Power and Square Root
    TEST_ASSERT(GoknarMath::Abs(GoknarMath::Pow(2.f, 3.f) - 8.f) < EPSILON, "GoknarMath::Pow");
    TEST_ASSERT(GoknarMath::Abs(GoknarMath::Sqrt(16.f) - 4.f) < EPSILON, "GoknarMath::Sqrt");

    // 5. LookAt Transformation Calculation
    // This is a complex calculation. We check if the resulting view matrix 
    // transforms the target point to the negative Z axis (standard view space).
    Matrix viewMat;
    Vector3 eye(0.f, 0.f, 10.f);
    Vector3 target(0.f, 0.f, 0.f);
    Vector3 up(0.f, 1.f, 0.f);

    GoknarMath::LookAt(viewMat, eye, target, up);

    // In view space, the target (0,0,0) should be in front of the camera.
    // Since camera is at (0,0,10) looking at (0,0,0), the target's view-space Z should be -10.
    Vector4 targetWorld(0.f, 0.f, 0.f, 1.f);
    Vector4 targetView = viewMat * targetWorld;

    TEST_ASSERT(GoknarMath::Abs(targetView.z - (-10.f)) < 0.001f, "LookAt Transformation Calculation");
}