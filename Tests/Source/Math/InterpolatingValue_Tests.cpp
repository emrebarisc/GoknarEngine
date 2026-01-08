#include "Math/InterpolatingValue_Tests.h"

#include "TestsCore.h"

#include "Goknar/Math/MathDefines.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/InterpolatingValue.h"

void InterpolatingValue_Tests::Run()
{
    try 
    {
        RunBasicInterpolationTests();
        RunTwoDestinationInterpolationTests();
        RunRotatingInterpolationTests();

        GOKNAR_CORE_INFO("All InterpolatingValue test sequences completed.");
    }
    catch (...)
    {
        GOKNAR_CORE_ERROR("An unexpected error occurred during testing InterpolatingValue.");
    }
}

void InterpolatingValue_Tests::RunBasicInterpolationTests()
{
    // 1. Initialization
    InterpolatingValue<float> val;
    val.Reset(0.f);
    TEST_ASSERT(val.current == 0.f && val.destination == 0.f, "InterpolatingValue_Tests: Initialization");

    // 2. Simple Linear Interpolation
    // Move from 0.0 to 10.0 with speed 1.0
    val.UpdateDestination(10.f);
    val.speed = 1.0f; 

    // Tick 0.5 seconds (should reach halfway, 5.0)
    val.Tick(0.5f);
    TEST_ASSERT(GoknarMath::Abs(val.current - 5.0f) < EPSILON, "InterpolatingValue_Tests: Tick (50%)");
    TEST_ASSERT(val.interpolationValue == 0.5f, "InterpolatingValue_Tests: Alpha Check");

    // Tick another 0.5 seconds (should reach destination, 10.0)
    val.Tick(0.5f);
    TEST_ASSERT(GoknarMath::Abs(val.current - 10.0f) < EPSILON, "InterpolatingValue_Tests: Tick (100%)");
    TEST_ASSERT(val.interpolationValue == 1.0f, "InterpolatingValue_Tests: Alpha Cap");

    // Overshoot Tick (should stay at 10.0)
    val.Tick(1.0f);
    TEST_ASSERT(GoknarMath::Abs(val.current - 10.0f) < EPSILON, "InterpolatingValue_Tests: Overshoot Clamp");
}

void InterpolatingValue_Tests::RunTwoDestinationInterpolationTests()
{
    // 1. Initialization
    TwoDestinationInterpolatedValue<float> val;
    val.Reset(0.f);
    
    // 2. Two-Stage Interpolation
    // Dest 1: 10.0, Dest 2: 20.0
    val.UpdateDestination(10.f, 20.f);
    val.speed = 1.0f;    // Speed for 0->1
    val.outSpeed = 1.0f; // Speed for 1->2

    // Tick 0.5s: Should be halfway to first destination (Lerp 0->10 at 0.5)
    val.Tick(0.5f);
    TEST_ASSERT(GoknarMath::Abs(val.current - 5.0f) < EPSILON, "InterpolatingValue_Tests: TwoDestination Stage 1 (50%)");

    // Tick 0.5s: Should reach first destination (Lerp 0->10 at 1.0)
    val.Tick(0.5f);
    TEST_ASSERT(GoknarMath::Abs(val.current - 10.0f) < EPSILON, "InterpolatingValue_Tests: TwoDestination Stage 1 (100%)");

    // Tick 0.5s: Should be halfway to second destination (Lerp 10->20 at 0.5 relative)
    // interpolationValue should be 1.5
    val.Tick(0.5f);
    TEST_ASSERT(GoknarMath::Abs(val.current - 15.0f) < EPSILON, "InterpolatingValue_Tests: TwoDestination Stage 2 (50%)");

    // Tick 0.5s: Should reach second destination
    val.Tick(0.5f);
    TEST_ASSERT(GoknarMath::Abs(val.current - 20.0f) < EPSILON, "InterpolatingValue_Tests: TwoDestination Stage 2 (100%)");
}

void InterpolatingValue_Tests::RunRotatingInterpolationTests()
{
    // RotatingInterpolatedValue assumes T is Vector3 (or compatible)
    RotatingInterpolatedValue<Vector3> val;

    // Initial state is ForwardVector (1,0,0) by default constructor
    // Set speed to 1.0
    val.speed = 1.0f;

    // 1. Rotate 90 Degrees around Z (Up)
    // Target: (0, 1, 0) - assuming Z is Up or Y is Left depending on coords.
    // The class calculates atan2(y, x). 
    // Start: (1, 0) -> Angle 0.
    // End: (0, 1) -> Angle PI/2.
    // Delta Angle: PI/2.
    Vector3 target(0.f, 1.f, 0.f);
    val.UpdateDestination(target);

    // Tick 0.5s (Halfway) -> Angle should be PI/4 (45 degrees)
    val.Tick(0.5f);

    // At 45 deg, x and y should be approx 0.707
    TEST_ASSERT(GoknarMath::Abs(val.current.x - 0.7071f) < 0.001f &&
        GoknarMath::Abs(val.current.y - 0.7071f) < 0.001f, "InterpolatingValue_Tests: RotatingInterpolatedValue 45 deg");

    // Tick 0.5s (Complete) -> Angle should be PI/2 (90 degrees)
    val.Tick(0.5f);

    // Should match target (0, 1, 0) close enough
    TEST_ASSERT(GoknarMath::Abs(val.current.x) < EPSILON &&
        GoknarMath::Abs(val.current.y - 1.0f) < EPSILON, "InterpolatingValue_Tests: RotatingInterpolatedValue 90 deg");

    // 2. Shortest Path Logic Check
    // If we are at 10 degrees and go to 350 degrees (-10), it should go the short way (-20 delta)
    // rather than the long way (+340 delta).
    // Note: The logic inside RotatingInterpolatedValue handles PI wrapping.

    // Reset to Forward (0 deg)
    val.Reset(Vector3(1.f, 0.f, 0.f));

    // Target slightly below X axis (e.g. -10 degrees / 350 degrees)
    // cos(-10) = 0.9848, sin(-10) = -0.1736
    Vector3 targetNegative(0.9848f, -0.1736f, 0.f);
    val.UpdateDestination(targetNegative);

    // The calculated destinationAngle should be negative (approx -0.174 rad)
    // If logic is correct, interpolationValue += deltaTime * speed will multiply this negative angle.
    val.Tick(0.5f);

    // We expect y to be negative (rotating clockwise/down)
    TEST_ASSERT(val.current.y < 0.f, "InterpolatingValue_Tests: RotatingInterpolatedValue Shortest Path Direction");
}