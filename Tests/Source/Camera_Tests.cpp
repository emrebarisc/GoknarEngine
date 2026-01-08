#include "Camera_Tests.h"

#include "TestsCore.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/Matrix.h"

void Camera_Tests::Run()
{
    try 
    {
        RunInitializationTests();
        RunMovementTests();
        RunViewMatrixTests();
        RunProjectionTests();

        GOKNAR_CORE_INFO("All Camera test sequences completed.");
    }
    catch (...)
    {
        GOKNAR_CORE_ERROR("An unexpected error occurred during testing Camera.");
    }
}

void Camera_Tests::RunInitializationTests()
{
    Camera* camera = new Camera();
    camera->Init();

    TEST_ASSERT(camera->GetPosition() == Vector3::ZeroVector, "Camera Default Position");
    TEST_ASSERT(camera->GetForwardVector() == Vector3::ForwardVector, "Camera Default Forward");
    TEST_ASSERT(camera->GetProjection() == CameraProjection::Perspective, "Camera Default Projection Mode");

    Vector3 pos(10.f, 10.f, 10.f);
    Vector3 fwd(1.f, 0.f, 0.f);
    Vector3 up(0.f, 0.f, 1.f);
    Camera* cameraCopy = new Camera(pos, fwd, up);

    TEST_ASSERT(cameraCopy->GetPosition() == pos, "Camera Param Constructor Position");
    TEST_ASSERT(cameraCopy->GetForwardVector() == fwd, "Camera Param Constructor Forward");
    TEST_ASSERT(cameraCopy->GetUpVector() == up, "Camera Param Constructor Up");

    camera->Destroy();
    cameraCopy->Destroy();
}

void Camera_Tests::RunMovementTests()
{
    Camera* camera = new Camera();

    camera->SetPosition(Vector3::ZeroVector);
    camera->SetVectors(Vector3(1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f), Vector3(0.f, 0.f, 1.f));
    // Assuming Standard Basis: Forward(1,0,0), Left(0,1,0), Up(0,0,1) based on set vectors

    // 1. Move Forward
    camera->MoveForward(10.f);
    TEST_ASSERT(GoknarMath::Abs(camera->GetPosition().x - 10.f) < EPSILON, "Camera MoveForward");

    // 2. Move Right (Negative Left)
    // If Left is (0,1,0), Right is (0,-1,0). 
    // Note: Verify "MoveRight" implementation direction in Camera.cpp. 
    // Usually MoveRight(x) implies position += RightVector * x. 
    // Since LeftVector is stored, Right is -LeftVector.
    camera->MoveRight(5.f);
    // If Left is (0,1,0), Right is (0,-1,0). Position should be y = 5.
    TEST_ASSERT(GoknarMath::Abs(camera->GetPosition().y - 5.f) < EPSILON, "Camera MoveRight");

    // 3. Move Upward
    camera->MoveUpward(2.f);
    TEST_ASSERT(GoknarMath::Abs(camera->GetPosition().z - 2.f) < EPSILON, "Camera MoveUpward");

    camera->Destroy();
}

void Camera_Tests::RunViewMatrixTests()
{
    Camera* camera = new Camera();
    
    // 1. Identity View
    // Position at origin, looking down Forward, Up is Up.
    // The View Matrix should be close to Identity (or coordinate system basis swap depending on engine convention).
    camera->SetPosition(Vector3::ZeroVector);
    camera->SetVectors(Vector3::ForwardVector, Vector3::LeftVector, Vector3::UpVector); 
    
    // Note: Exact matrix check depends on Goknar's coordinate system (LH vs RH, Z-up vs Y-up).
    // However, the ViewMatrix * ForwardVector should transform the forward direction to View Space Forward (usually -Z or +Z).
    
    // 2. LookAt Logic
    // Place camera at (10, 0, 0) looking at (0, 0, 0).
    // Forward Vector becomes (-1, 0, 0).
    camera->SetPosition(Vector3(10.f, 0.f, 0.f));
    camera->SetForwardVector(Vector3(-1.f, 0.f, 0.f)); // Looking back at origin
    
    // The View Matrix transforms World Space to Camera Space.
    // A point at (10, 0, 0) (the camera pos) should map to (0, 0, 0).
    Matrix viewMat = camera->GetViewMatrix();
    Vector4 camPosWorld(10.f, 0.f, 0.f, 1.f);
    Vector4 camPosView = viewMat * camPosWorld;

    TEST_ASSERT(GoknarMath::Abs(camPosView.x) < EPSILON &&
                GoknarMath::Abs(camPosView.y) < EPSILON &&
                GoknarMath::Abs(camPosView.z) < EPSILON, "View Matrix Translation Check");

    // 3. Update View Matrix Flag
    // Changing position with updateViewMatrix = false should NOT change the matrix immediately.
    Matrix oldMatrix = camera->GetViewMatrix();
    camera->SetPosition(Vector3(20.f, 0.f, 0.f), false);
    TEST_ASSERT(camera->GetViewMatrix() == oldMatrix, "SetPosition Deferred Update");
    
    // Force update (usually done via Update() or manually calling a setter with true)
    camera->SetPosition(Vector3(20.f, 0.f, 0.f), true);
    TEST_ASSERT(!(camera->GetViewMatrix() == oldMatrix), "SetPosition Immediate Update");

    camera->Destroy();
}

void Camera_Tests::RunProjectionTests()
{
    Camera* camera = new Camera();
    
    // 1. Aspect Ratio
    camera->SetImageWidth(800);
    camera->SetImageHeight(600);
    // Near plane should update automatically
    Vector4 nearPlane = camera->GetNearPlane();
    float ratio = 800.f / 600.f;
    
    // Width of near plane (Right - Left) / Height of near plane (Top - Bottom) should approx equal aspect ratio
    float width = nearPlane.y - nearPlane.x; // Right - Left
    float height = nearPlane.w - nearPlane.z; // Top - Bottom
    TEST_ASSERT(GoknarMath::Abs((width / height) - ratio) < EPSILON, "Camera Aspect Ratio Update");

    // 2. FOV Change
    Matrix oldProj = camera->GetProjectionMatrix();
    camera->SetFOV(90.f); 
    Matrix newProj = camera->GetProjectionMatrix();

    // Changing FOV must alter the projection matrix (specifically scale factors [0] and [5])
    TEST_ASSERT(!(oldProj == newProj), "SetFOV Updates Projection Matrix");

    // 3. Orthographic Switch
    camera->SetProjection(CameraProjection::Orthographic);
    TEST_ASSERT(camera->GetProjection() == CameraProjection::Orthographic, "SetProjection Enum");
    // Orthographic matrices usually have m[15] == 1.0, Perspective usually m[15] == 0.0
    // This depends on the specific matrix construction in Goknar, but the matrix should definitely change.
    TEST_ASSERT(!(newProj == camera->GetProjectionMatrix()), "Switching to Orthographic Updates Matrix");

    camera->Destroy();
}