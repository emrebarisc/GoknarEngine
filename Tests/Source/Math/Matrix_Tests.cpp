#include "Math/Matrix_Tests.h"

#include "TestsCore.h"

#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/Matrix.h"

void Matrix_Tests::Run()
{
    try 
    {
        RunMatrixTests();
        RunMatrix3x3Tests();

        GOKNAR_CORE_INFO("All Matrix test sequences completed.");
    }
    catch (...)
    {
        GOKNAR_CORE_ERROR("An unexpected error occurred during Matrix testing.");
    }
}

void Matrix_Tests::RunMatrixTests()
{
    // 1. Initialization and Identity
    Matrix mId = Matrix::IdentityMatrix;
    TEST_ASSERT(GoknarMath::Abs(mId[0] - 1.f) < EPSILON && 
                GoknarMath::Abs(mId[5] - 1.f) < EPSILON && 
                GoknarMath::Abs(mId[10] - 1.f) < EPSILON && 
                GoknarMath::Abs(mId[15] - 1.f) < EPSILON, "Matrix_Tests: Matrix Identity Constructor");
    
    TEST_ASSERT(GoknarMath::Abs(mId[1]) < EPSILON, "Matrix_Tests: Matrix Identity Off-Diagonal");

    // 2. Transpose
    // 0  1  2  3
    // 4  5  6  7
    // 8  9 10 11
    //12 13 14 15
    Matrix mSeq(0.f, 1.f, 2.f, 3.f,
                4.f, 5.f, 6.f, 7.f,
                8.f, 9.f, 10.f, 11.f,
                12.f, 13.f, 14.f, 15.f);

    Matrix mTrans = mSeq.GetTranspose();

    // Index 1 (Row 0, Col 1) should become Index 4 (Row 1, Col 0)
    TEST_ASSERT(GoknarMath::Abs(mTrans[4] - 1.f) < EPSILON && 
                GoknarMath::Abs(mTrans[1] - 4.f) < EPSILON, "Matrix_Tests: Matrix Transpose");

    // 3. Matrix Multiplication (Identity)
    Matrix mMulId = mSeq * Matrix::IdentityMatrix;
    TEST_ASSERT(mMulId == mSeq, "Matrix_Tests: Matrix * Identity");

    // 4. Matrix Multiplication (Scaling)
    Matrix mScale2 = Matrix::GetScalingMatrix(Vector3(2.f, 2.f, 2.f));
    Matrix mScaled = mId * mScale2; 
    TEST_ASSERT(GoknarMath::Abs(mScaled[0] - 2.f) < EPSILON && 
                GoknarMath::Abs(mScaled[5] - 2.f) < EPSILON, "Matrix_Tests: Matrix Multiplication (Scaling)");

    // 5. Translation Creation and Access
    Vector3 pos(10.f, 20.f, 30.f);
    Matrix mTransMat = Matrix::GetPositionMatrix(pos);
    
    // Check internal storage (Column 3 for standard 4x4, usually indices 3, 7, 11)
    TEST_ASSERT(GoknarMath::Abs(mTransMat[3] - 10.f) < EPSILON &&
                GoknarMath::Abs(mTransMat[7] - 20.f) < EPSILON &&
                GoknarMath::Abs(mTransMat[11] - 30.f) < EPSILON, "Matrix_Tests: GetPositionMatrix Internal Values");

    Vector3 extractedPos = mTransMat.GetTranslation();
    TEST_ASSERT(extractedPos == pos, "Matrix_Tests: GetTranslation Extraction");

    // 6. Matrix * Vector4 Transformation
    // Translate (10, 0, 0) applied to Point (0, 0, 0) -> (10, 0, 0)
    // Note: Point w component must be 1.0
    Vector4 vPoint(0.f, 0.f, 0.f, 1.f);
    Matrix mMoveX = Matrix::GetPositionMatrix(Vector3(10.f, 0.f, 0.f));
    Vector4 vTransformed = mMoveX * vPoint;

    TEST_ASSERT(GoknarMath::Abs(vTransformed.x - 10.f) < EPSILON, "Matrix_Tests: Matrix * Vector4 (Translation)");

    // 7. Matrix Inverse (Simple Translation)
    // Inverse of Move(10, 0, 0) is Move(-10, 0, 0)
    Matrix mInv = mMoveX.GetInverse();
    TEST_ASSERT(GoknarMath::Abs(mInv[3] - -10.f) < EPSILON, "Matrix_Tests: Matrix GetInverse (Translation)");

    // Multiply Matrix by its Inverse -> Identity
    Matrix mIdentityCheck = mMoveX * mInv;
    TEST_ASSERT(mIdentityCheck.Equals(Matrix::IdentityMatrix), "Matrix_Tests: Matrix * Inverse == Identity");

    // 8. Rotation (Axis Angle)
    // Rotate 90 degrees (PI/2) around Y axis
    // Vector (1, 0, 0) -> (0, 0, -1) (Right Hand Rule usually, depending on coord system)
    // Or (0, 0, 1) depending on implementation details.
    // Let's check the matrix values directly.
    // Cos(90) = 0, Sin(90) = 1
    // Y-Rot Matrix:
    //  0  0  1  0
    //  0  1  0  0
    // -1  0  0  0
    //  0  0  0  1
    // (Signs of sin depend on implementation)
    
    Matrix mRotY = Matrix::GetRotationMatrixAboutAnAxis(Vector3(0.f, 1.f, 0.f), PI / 2.f);
    Vector4 vForward(1.f, 0.f, 0.f, 1.f);
    Vector4 vRotated = mRotY * vForward;

    // Based on standard math, rotating +X 90deg around +Y results in -Z or +Z depending on handedness.
    // We assert that X is 0 and Z has magnitude 1.
    TEST_ASSERT(GoknarMath::Abs(vRotated.x) < EPSILON && 
                GoknarMath::Abs(vRotated.z) > 0.9f, "Matrix_Tests: Matrix Rotation (Y-Axis 90 deg)");

    // 9. Full Transformation Matrix
    // Combine Scale(2), Rotate(Identity), Translate(1,1,1)
    Vector3 tPos(1.f, 1.f, 1.f);
    Vector3 tScale(2.f, 2.f, 2.f);
    Quaternion tRot = Quaternion::Identity;
    
    Matrix mTransform = Matrix::GetTransformationMatrix(tRot, tPos, tScale);
    
    // Test Scaling part (diagonal)
    TEST_ASSERT(GoknarMath::Abs(mTransform[0] - 2.f) < EPSILON, "Matrix_Tests: Transformation Matrix Scale Part");
    
    // Test Translation part
    TEST_ASSERT(GoknarMath::Abs(mTransform[3] - 1.f) < EPSILON, "Matrix_Tests: Transformation Matrix Translation Part");

    // 10. Axis Accessors
    // Identity Matrix: Forward=(0,0,1) or (1,0,0) depending on engine convention?
    // Looking at header: GetForwardVector returns indices [0, 4, 8] which is Column 0.
    // GetLeftVector returns indices [1, 5, 9] which is Column 1.
    // GetUpVector returns indices [2, 6, 10] which is Column 2.
    // NOTE: Usually Forward is Z (Col 2) in many engines, but code says Forward is Col 0 (X). 
    // We test based on code definition.
    Vector3 fwd = mId.GetForwardVector();
    TEST_ASSERT(GoknarMath::Abs(fwd.x - 1.f) < EPSILON, "Matrix_Tests: GetForwardVector (Identity)");
}

void Matrix_Tests::RunMatrix3x3Tests()
{
    // 1. Initialization
    Matrix3x3 m3Id = Matrix3x3::IdentityMatrix;
    TEST_ASSERT(GoknarMath::Abs(m3Id[0] - 1.f) < EPSILON && 
                GoknarMath::Abs(m3Id[4] - 1.f) < EPSILON && 
                GoknarMath::Abs(m3Id[8] - 1.f) < EPSILON, "Matrix_Tests: Matrix3x3 Identity");

    // 2. Vector3 Multiplication
    // Matrix: Scale X by 2
    // 2 0 0
    // 0 1 0
    // 0 0 1
    Matrix3x3 mScaleX(2.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f);
    Vector3 v(1.f, 1.f, 1.f);
    Vector3 vRes = mScaleX * v;

    TEST_ASSERT(GoknarMath::Abs(vRes.x - 2.f) < EPSILON && 
                GoknarMath::Abs(vRes.y - 1.f) < EPSILON, "Matrix_Tests: Matrix3x3 * Vector3");
    
    // 3. Transpose
    Matrix3x3 mSeq(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Matrix3x3 mTrans = mSeq.GetTranspose();
    TEST_ASSERT(GoknarMath::Abs(mTrans[1] - 4.f) < EPSILON, "Matrix_Tests: Matrix3x3 Transpose");
}