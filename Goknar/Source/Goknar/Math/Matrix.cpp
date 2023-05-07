#include "pch.h"

#include "Matrix.h"

const Matrix2x2 Matrix2x2::ZeroMatrix = Matrix2x2(0.f);
const Matrix2x2 Matrix2x2::IdentityMatrix = Matrix2x2(  1.f, 0.f, 
                                                        0.f, 1.f );

const Matrix3x3 Matrix3x3::ZeroMatrix = Matrix3x3(0.f);
const Matrix3x3 Matrix3x3::IdentityMatrix = Matrix3x3(  1.f, 0.f, 0.f,
                                                        0.f, 1.f, 0.f,
                                                        0.f, 0.f, 1.f);

const Matrix Matrix::ZeroMatrix = Matrix(0.f);
const Matrix Matrix::IdentityMatrix = Matrix( 1.f, 0.f, 0.f, 0.f,
                                              0.f, 1.f, 0.f, 0.f,
                                              0.f, 0.f, 1.f, 0.f,
                                              0.f, 0.f, 0.f, 1.f );

Matrix2x2 Matrix2x2::GetInverse() const
{
    float oneOverDet = 1 / (m[0] * m[3] - m[1] * m[2]);
    Matrix2x2 lhs(m[3], -m[1], -m[2], m[0]);

    return oneOverDet * lhs;
}

void Matrix3x3::GetInverse(Matrix3x3& inverse) const
{
    float t4 = m[0] * m[4];
    float t6 = m[0] * m[5];
    float t8 = m[1] * m[3];
    float t10 = m[2] * m[3];
    float t12 = m[1] * m[6];
    float t14 = m[2] * m[6];

    // Calculate the determinant
    float t16 = (t4 * m[8] - t6 * m[7] - t8 * m[8] +
        t10 * m[7] + t12 * m[5] - t14 * m[4]);

    // Make sure the determinant is non-zero.
    if (t16 == 0.0f)
    {
        return;
    }

    float t17 = 1.f / t16;

    inverse.m[0] = (m[4] * m[8] - m[5] * m[7]) * t17;
    inverse.m[1] = -(m[1] * m[8] - m[2] * m[7]) * t17;
    inverse.m[2] = (m[1] * m[5] - m[2] * m[4]) * t17;
    inverse.m[3] = -(m[3] * m[8] - m[5] * m[6]) * t17;
    inverse.m[4] = (m[0] * m[8] - t14) * t17;
    inverse.m[5] = -(t6 - t10) * t17;
    inverse.m[6] = (m[3] * m[7] - m[4] * m[6]) * t17;
    inverse.m[7] = -(m[0] * m[7] - t12) * t17;
    inverse.m[8] = (t4 - t8) * t17;
}

Matrix3x3 Matrix3x3::GetInverse() const
{
    Matrix3x3 inverse;

    GetInverse(inverse);

    return inverse;
}

Matrix3x3 Matrix3x3::GetTranspose() const
{
    Matrix3x3 out;

    out.m[0] = m[0];
    out.m[1] = m[3];
    out.m[2] = m[6];

    out.m[3] = m[1];
    out.m[4] = m[4];
    out.m[5] = m[7];

    out.m[6] = m[2];
    out.m[7] = m[5];
    out.m[8] = m[8];

    return out;
}

Matrix Matrix::GetInverse() const
{
    Matrix out;

    out.m[0] =  m[5]  * m[10] * m[15] -
                m[5]  * m[11] * m[14] -
                m[9]  * m[6]  * m[15] +
                m[9]  * m[7]  * m[14] +
                m[13] * m[6]  * m[11] -
                m[13] * m[7]  * m[10];

    out.m[4] = -m[4]  * m[10] * m[15] +
                m[4]  * m[11] * m[14] +
                m[8]  * m[6]  * m[15] -
                m[8]  * m[7]  * m[14] -
                m[12] * m[6]  * m[11] +
                m[12] * m[7]  * m[10];

    out.m[8] =  m[4]  * m[9] * m[15] -
                m[4]  * m[11] * m[13] -
                m[8]  * m[5] * m[15] +
                m[8]  * m[7] * m[13] +
                m[12] * m[5] * m[11] -
                m[12] * m[7] * m[9];

    out.m[12] = -m[4]  * m[9] * m[14] +
                 m[4]  * m[10] * m[13] +
                 m[8]  * m[5] * m[14] -
                 m[8]  * m[6] * m[13] -
                 m[12] * m[5] * m[10] +
                 m[12] * m[6] * m[9];

    out.m[1] = -m[1]  * m[10] * m[15] +
                m[1]  * m[11] * m[14] +
                m[9]  * m[2] * m[15] -
                m[9]  * m[3] * m[14] -
                m[13] * m[2] * m[11] +
                m[13] * m[3] * m[10];

    out.m[5] =  m[0]  * m[10] * m[15] -
                m[0]  * m[11] * m[14] -
                m[8]  * m[2] * m[15] +
                m[8]  * m[3] * m[14] +
                m[12] * m[2] * m[11] -
                m[12] * m[3] * m[10];

    out.m[9] = -m[0]  * m[9] * m[15] +
                m[0]  * m[11] * m[13] +
                m[8]  * m[1] * m[15] -
                m[8]  * m[3] * m[13] -
                m[12] * m[1] * m[11] +
                m[12] * m[3] * m[9];

    out.m[13] = m[0]  * m[9] * m[14] -
                m[0]  * m[10] * m[13] -
                m[8]  * m[1] * m[14] +
                m[8]  * m[2] * m[13] +
                m[12] * m[1] * m[10] -
                m[12] * m[2] * m[9];

    out.m[2] =  m[1]  * m[6] * m[15] -
                m[1]  * m[7] * m[14] -
                m[5]  * m[2] * m[15] +
                m[5]  * m[3] * m[14] +
                m[13] * m[2] * m[7] -
                m[13] * m[3] * m[6];

    out.m[6] = -m[0]  * m[6] * m[15] +
                m[0]  * m[7] * m[14] +
                m[4]  * m[2] * m[15] -
                m[4]  * m[3] * m[14] -
                m[12] * m[2] * m[7] +
                m[12] * m[3] * m[6];

    out.m[10] = m[0]  * m[5] * m[15] -
                m[0]  * m[7] * m[13] -
                m[4]  * m[1] * m[15] +
                m[4]  * m[3] * m[13] +
                m[12] * m[1] * m[7] -
                m[12] * m[3] * m[5];

    out.m[14] = -m[0]  * m[5] * m[14] +
                 m[0]  * m[6] * m[13] +
                 m[4]  * m[1] * m[14] -
                 m[4]  * m[2] * m[13] -
                 m[12] * m[1] * m[6] +
                 m[12] * m[2] * m[5];

    out.m[3] = -m[1] * m[6] * m[11] +
                m[1] * m[7] * m[10] +
                m[5] * m[2] * m[11] -
                m[5] * m[3] * m[10] -
                m[9] * m[2] * m[7] +
                m[9] * m[3] * m[6];

    out.m[7] =  m[0] * m[6] * m[11] -
                m[0] * m[7] * m[10] -
                m[4] * m[2] * m[11] +
                m[4] * m[3] * m[10] +
                m[8] * m[2] * m[7] -
                m[8] * m[3] * m[6];

    out.m[11] = -m[0] * m[5] * m[11] +
                 m[0] * m[7] * m[9] +
                 m[4] * m[1] * m[11] -
                 m[4] * m[3] * m[9] -
                 m[8] * m[1] * m[7] +
                 m[8] * m[3] * m[5];

    out.m[15] = m[0] * m[5] * m[10] -
                m[0] * m[6] * m[9] -
                m[4] * m[1] * m[10] +
                m[4] * m[2] * m[9] +
                m[8] * m[1] * m[6] -
                m[8] * m[2] * m[5];

    float det = m[0] * out.m[0] + m[1] * out.m[4] + m[2] * out.m[8] + m[3] * out.m[12];

    if (det == 0)
    {
        std::cout << *this << std::endl;
        std::cout << out << std::endl;
        throw std::runtime_error("Error: Determinant is zero.");
    }

    det = 1.f / det;

    for (int i = 0; i < 16; ++i)
    {
        out.m[i] *= det;
    }

    return out;
}

Matrix Matrix::GetTranspose() const
{
    Matrix out;

    out.m[0] = m[0];
    out.m[1] = m[4];
    out.m[2] = m[8];
    out.m[3] = m[12];

    out.m[4] = m[1];
    out.m[5] = m[5];
    out.m[6] = m[9];
    out.m[7] = m[13];

    out.m[8] = m[2];
    out.m[9] = m[6];
    out.m[10] = m[10];
    out.m[11] = m[14];

    out.m[12] = m[3];
    out.m[13] = m[5];
    out.m[14] = m[11];
    out.m[15] = m[15];

    return out;
}

Matrix3x3 Matrix::GetUpper3x3() const
{
    Matrix3x3 out;

    out.m[0] = m[0];
    out.m[1] = m[1];
    out.m[2] = m[2];

    out.m[3] = m[4];
    out.m[4] = m[5];
    out.m[5] = m[6];

    out.m[6] = m[8];
    out.m[7] = m[9];
    out.m[8] = m[10];

    return out;
}