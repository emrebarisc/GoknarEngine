#include "pch.h"

#include "Matrix.h"

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

Matrix Matrix::GetUpper3x3() const
{
    Matrix out;

    out.m[0] = m[0];
    out.m[1] = m[1];
    out.m[2] = m[2];
    out.m[3] = 0.f;

    out.m[4] = m[4];
    out.m[5] = m[5];
    out.m[6] = m[6];
    out.m[7] = 0.f;

    out.m[8] = m[8];
    out.m[9] = m[9];
    out.m[10] = m[10];
    out.m[11] = 0.f;

    out.m[12] = m[12];
    out.m[13] = m[13];
    out.m[14] = m[14];
    out.m[15] = 0.f;

    return out;
}