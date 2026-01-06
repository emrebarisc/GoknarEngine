#include "Math/GoknarMath_Tests.h"

#include "TestsCore.h"

#include "Goknar/Math/GoknarMath.h"

void GoknarMath_Tests::Run()
{
    try 
    {
        RunMacroTests();
        RunMathUtilsTests();
        RunVector2Tests();
        RunVector3Tests();
        RunVector4Tests();

        GOKNAR_CORE_INFO("All test sequences completed.");
    }
    catch (...)
    {

        GOKNAR_CORE_ERROR("An unexpected error occurred during testing.");
    }
}

void GoknarMath_Tests::RunMacroTests()
{
    TEST_ASSERT(mathMax(5, 10) == 10, "TEST FAILED: mathMax(5, 10)");
    TEST_ASSERT(mathMin(5, 10) == 5, "TEST FAILED: mathMin(5, 10)");
    TEST_ASSERT(mathClamp(15, 0, 10) == 10, "TEST FAILED: mathClamp upper bound");
    TEST_ASSERT(mathClamp(-5, 0, 10) == 0, "TEST FAILED: mathClamp lower bound");
    TEST_ASSERT(mathAbs(-5.5f) == 5.5f, "TEST FAILED: mathAbs negative");
}

void GoknarMath_Tests::RunMathUtilsTests()
{
    TEST_ASSERT(GoknarMath::Lerp(0.0f, 10.0f, 0.5f) == 5.0f, "TEST FAILED: Lerp 50%");
    
    TEST_ASSERT(GoknarMath::IsNanOrInf(std::nanf("")), "TEST FAILED: IsNanOrInf NaN check");
    TEST_ASSERT(!GoknarMath::IsNanOrInf(5.0f), "TEST FAILED: IsNanOrInf valid number check");

    TEST_ASSERT(GoknarMath::Min(10, 20) == 10, "TEST FAILED: GoknarMath::Min");
    TEST_ASSERT(GoknarMath::Max(10, 20) == 20, "TEST FAILED: GoknarMath::Max");
    TEST_ASSERT(GoknarMath::Floor(2.9f) == 2.0f, "TEST FAILED: GoknarMath::Floor");
    TEST_ASSERT(GoknarMath::FloorToInt(2.9f) == 2, "TEST FAILED: GoknarMath::FloorToInt");
    
    float frac = GoknarMath::Frac(1.75f);
    TEST_ASSERT(frac > 0.74f && frac < 0.76f, "TEST FAILED: GoknarMath::Frac (1.75 -> 0.75)");
}

void GoknarMath_Tests::RunVector2Tests()
{
    Vector2 v1(3.0f, 4.0f);
    Vector2 v2(1.0f, 2.0f);

    TEST_ASSERT(v1.Length() == 5.0f, "TEST FAILED: Vector2 Length (3,4) = 5");
    
    Vector2 vSum = v1 + v2;
    TEST_ASSERT(vSum.x == 4.0f && vSum.y == 6.0f, "TEST FAILED: Vector2 Addition");

    float dot = Vector2::Dot(v1, v2);
    TEST_ASSERT(dot == 11.0f, "TEST FAILED: Vector2 Dot Product");

    v1.Clamp(0.0f, 3.5f);
    TEST_ASSERT(v1.y == 3.5f, "TEST FAILED: Vector2 Clamp");
}

void GoknarMath_Tests::RunVector3Tests()
{
    Vector3 v1(1.0f, 0.0f, 0.0f);
    Vector3 v2(0.0f, 1.0f, 0.0f);

    Vector3 cross = Vector3::Cross(v1, v2);
    TEST_ASSERT(cross.x == 0.0f && cross.y == 0.0f && cross.z == 1.0f, "TEST FAILED: Vector3 Cross Product");

    Vector3 v3(5.0f, 0.0f, 0.0f);
    Vector3 norm = v3.GetNormalized();
    TEST_ASSERT(norm.x == 1.0f && norm.Length() == 1.0f, "TEST FAILED: Vector3 Normalize");

    float dist = Vector3::Distance(Vector3(0,0,0), Vector3(0,10,0));
    TEST_ASSERT(dist == 10.0f, "TEST FAILED: Vector3 Distance");

    Vector3 v4 = v1 * 5.0f;
    TEST_ASSERT(v4.x == 5.0f, "TEST FAILED: Vector3 Scalar Multiplication");
}

void GoknarMath_Tests::RunVector4Tests()
{
    Vector4 v1(1, 2, 3, 4);
    Vector4 v2(1, 1, 1, 1);
    
    Vector4 res = v1 - v2;
    TEST_ASSERT(res.x == 0 && res.w == 3, "TEST FAILED: Vector4 Subtraction");
    
    TEST_ASSERT(v1.ToString().find("1.0") != std::string::npos, "TEST FAILED: Vector4 ToString check");
}