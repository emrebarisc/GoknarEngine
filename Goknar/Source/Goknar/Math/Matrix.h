#ifndef __MATRIX_H__
#define __MATRIX_H__

#include "Goknar/Core.h"

#include "Math/GoknarMath.h"
#include "Math/Quaternion.h"

/*
    dataArray IS NOT TESTED!!!
*/
class GOKNAR_API Matrix2x2
{
public:
    Matrix2x2()
    {
		m[0] = m[1] = m[2] = m[3] = 0.f;
    }

    Matrix2x2(float value)
    {
		m[0] = m[1] = m[2] = m[3] = value;
    }

    Matrix2x2(float val1, float val2,
              float val3, float val4)
    {
        m[0] = val1;
        m[1] = val2;
        m[2] = val3;
        m[3] = val4;
    }

    Matrix2x2(const float dataArray[4])
    {
        // For warning fix......?
        m[0] = 0.f;
        for (int i = 0; i < 4; i++)
        {
            m[i] = dataArray[i];
        }
    }

    Matrix2x2(const Matrix2x2& other)
    {
        // For warning fix......?
        m[0] = 0.f;
		if (this == &other) return;
        for (int i = 0; i < 4; i++)
        {
            m[i] = other.m[i];
        }
    }

    ~Matrix2x2()
    {

    }

    void operator=(const Matrix2x2& rhs)
    {
        if(this != &rhs)
        {
            for (int i = 0; i < 4; i++)
            {
                m[i] = rhs.m[i];
            }
        }
    }

    Matrix2x2 operator*(const Matrix2x2& rhs) const
    {
        Matrix2x2 out(0.f);

        out.m[0] = m[0] * rhs.m[0] + m[1] * rhs.m[2];
        out.m[1] = m[0] * rhs.m[1] + m[1] * rhs.m[3];
        out.m[2] = m[2] * rhs.m[0] + m[3] * rhs.m[1];
        out.m[3] = m[2] * rhs.m[1] + m[3] * rhs.m[3];

        return out;
    }

    Vector2 operator*(const Vector2& rhs) const
    {
        return Vector2(m[0] * rhs.x + m[1] * rhs.y, m[2] * rhs.x + m[3] * rhs.y);
    }

    friend Matrix2x2 operator*(float val, const Matrix2x2& rhs)
    {
        return Matrix2x2(rhs.m[0] * val, rhs.m[1] * val, rhs.m[2] * val, rhs.m[3] * val);
    }

    float operator[](int index)
    {
        if(index >= 0 && index <= 3)
        {
            return m[index];
        }

        return -1.f;
    }

    bool operator==(const Matrix2x2 &rhs)
    {
        for(unsigned int i = 0; i < 4; i++)
        {
            if(m[i] != rhs.m[i]) return false;
        }

        return true;
    }

    inline friend std::ostream& operator<<(std::ostream& out, const Matrix2x2& matrix)
    {
        return out << matrix.m[0] << ", " << matrix.m[1] << std::endl
                   << matrix.m[2] << ", " << matrix.m[3] << std::endl;
    }

    Matrix2x2 GetInverse() const;

    static const Matrix2x2 ZeroMatrix;
    static const Matrix2x2 IdentityMatrix;

    float m[4];
};

class GOKNAR_API Matrix3x3
{
public:
    Matrix3x3()
    {
        // For warning fix......?
        m[0] = 0.f;
        for (int i = 0; i < 9; i++)
        {
            m[i] = 0;
        }
    }

    Matrix3x3(float value)
    {
        // For warning fix......?
        m[0] = 0.f;
        for (int i = 0; i < 9; i++)
        {
            m[i] = value;
        }
    }

    Matrix3x3(  float val1, float val2, float val3,
                float val4, float val5, float val6, 
                float val7, float val8, float val9)
    {
        m[0] = val1;
        m[1] = val2;
        m[2] = val3;

        m[3] = val4;
        m[4] = val5;
        m[5] = val6;

        m[6] = val7;
        m[7] = val8;
        m[8] = val9;
    }

    Matrix3x3(const float matrix3x3[9])
    {
        // For warning fix......?
        m[0] = 0.f;
        for (int i = 0; i < 9; i++)
        {
            m[i] = matrix3x3[i];
        }
    }

    Matrix3x3(const Matrix3x3& other)
    {
        // For warning fix......?
        m[0] = 0.f;
        if (this != &other)
        {
            for (int i = 0; i < 9; i++)
            {
                m[i] = other.m[i];
            }
        }
    }

    ~Matrix3x3()
    {

    }

    inline static Matrix3x3 Lerp(const Matrix3x3& matrix1, const Matrix3x3& matrix2, float alpha)
    {
        return (matrix1 * (1.f - alpha)) + (matrix2 * alpha);
    }

    inline static Matrix3x3 GetRotationMatrixAroundXAxis(float xAngle)
    {
        const float cosTheta = cos(xAngle);
        const float sinTheta = sin(xAngle);

        return Matrix3x3(   1.f, 0.f, 0.f,
                            0.f, cosTheta, -sinTheta,
                            0.f, sinTheta, cosTheta);
    }

    inline static Matrix3x3 GetRotationMatrixAroundYAxis(float yAngle)
    {
        const float cosTheta = cos(yAngle);
        const float sinTheta = sin(yAngle);

        return Matrix3x3(cosTheta,    0.f, sinTheta,
                         0.f,         1.f, 0.f,
                         -sinTheta,   0.f, cosTheta);
    }
    
    inline static Matrix3x3 GetRotationMatrixAroundZAxis(float zAngle)
    {
        const float cosTheta = cos(zAngle);
        const float sinTheta = sin(zAngle);

        return Matrix3x3(   cosTheta, -sinTheta, 0.f,
                            sinTheta, cosTheta, 0.f,
                            0.f, 0.f, 1.f);
    }

    inline static Matrix3x3 GetPositionMatrix(const Vector2& position)
    {
        return Matrix3x3(   1.f, 0.f, position.x,
                            0.f, 1.f, position.y,
                            0.f, 0.f, 1.f);
    }

    inline static Matrix3x3 GetScalingMatrix(const Vector2& scaling)
    {
        return Matrix3x3(   scaling.x, 0.f, 0.f,
                            0.f, scaling.y, 0.f,
                            0.f, 0.f, 1.f);
    }

    void GetInverse(Matrix3x3& inverse) const;
    Matrix3x3 GetInverse() const;
    Matrix3x3 GetTranspose() const;

    inline Vector3 MultiplyTransposeBy(const Vector3& vector) const
    {
        return Vector3( vector.x * m[0] + vector.y * m[3] + vector.z * m[6],
                        vector.x * m[1] + vector.y * m[4] + vector.z * m[7],
                        vector.x * m[2] + vector.y * m[5] + vector.z * m[8]);
    }

    /**
     * Sets the matrix to be a skew symmetric matrix based on
     * the given vector. The skew symmetric matrix is the equivalent
     * of the vector product. So if a,b are vectors. a x b = A_s b
     * where A_s is the skew symmetric form of a.
     */
    void SetSkewSymmetric(const Vector3& vector)
    {
        m[0] = m[4] = m[8] = 0.f;
        m[1] = -vector.z;
        m[2] = vector.y;
        m[3] = vector.z;
        m[5] = -vector.x;
        m[6] = -vector.y;
        m[7] = vector.x;
    }

    Vector3 GetAxisVector(int i) const
    {
        return Vector3(m[i], m[i + 3], m[i + 6]);
    }

    void operator=(const Matrix3x3& rhs)
    {
        if (this != &rhs)
        {
            for (int i = 0; i < 9; i++)
            {
                m[i] = rhs.m[i];
            }
        }
    }

    Matrix3x3 operator+(const Matrix3x3& other)
    {
        Matrix3x3 result(*this);

        result.m[0] += other.m[0];
        result.m[1] += other.m[1];
        result.m[2] += other.m[2];

        result.m[3] += other.m[3];
        result.m[4] += other.m[4];
        result.m[5] += other.m[5];

        result.m[6] += other.m[6];
        result.m[7] += other.m[7];
        result.m[8] += other.m[8];

        return result;
    }

    Matrix3x3 operator-()
    {
        Matrix3x3 result(*this);

        for (int i = 0; i < 9; i++)
        {
            result.m[i] *= -1;
        }

        return result;
    }

    Vector3 operator*(const Vector3& rhs) const
    {
        Vector3 out;

        out.x = m[0] * rhs.x + m[1] * rhs.y + m[2] * rhs.z;
        out.y = m[3] * rhs.x + m[4] * rhs.y + m[5] * rhs.z;
        out.z = m[6] * rhs.x + m[7] * rhs.y + m[8] * rhs.z;

        return out;
    }

    Matrix3x3 operator*(float value) const
    {
        Matrix3x3 out(
            m[0] * value, m[1] * value, m[2] * value,
            m[3] * value, m[4] * value, m[5] * value,
            m[6] * value, m[7] * value, m[8] * value
        );

        return out;
    }

    Matrix3x3 operator*(const Matrix3x3& rhs) const
    {
        Matrix3x3 out(0.f);

        out.m[0] = m[0] * rhs.m[0] + m[1] * rhs.m[3] + m[2] * rhs.m[6];
        out.m[1] = m[0] * rhs.m[1] + m[1] * rhs.m[4] + m[2] * rhs.m[7];
        out.m[2] = m[0] * rhs.m[2] + m[1] * rhs.m[5] + m[2] * rhs.m[8];

        out.m[3] = m[3] * rhs.m[0] + m[4] * rhs.m[3] + m[5] * rhs.m[6];
        out.m[4] = m[3] * rhs.m[1] + m[4] * rhs.m[4] + m[5] * rhs.m[7];
        out.m[5] = m[3] * rhs.m[2] + m[4] * rhs.m[5] + m[5] * rhs.m[8];

        out.m[6] = m[6] * rhs.m[0] + m[7] * rhs.m[3] + m[8] * rhs.m[6];
        out.m[7] = m[6] * rhs.m[1] + m[7] * rhs.m[4] + m[8] * rhs.m[7];
        out.m[8] = m[6] * rhs.m[2] + m[7] * rhs.m[5] + m[8] * rhs.m[8];

        return out;
    }

    inline void operator*=(const Matrix3x3& rhs)
    {
        *this = (*this) * rhs;
    }

    float operator[](int index)
    {
        if (index >= 0 && index < 9)
        {
            return m[index];
        }

        return 0.f;
    }

    const float operator[](int index) const
    {
        if (index >= 0 && index < 9)
        {
            return m[index];
        }

        return 0.f;
    }

    bool operator==(const Matrix3x3& rhs)
    {
        for (unsigned int i = 0; i < 9; i++)
        {
            if (m[i] != rhs.m[i]) return false;
        }

        return true;
    }

    void operator+=(const Matrix3x3& rhs)
    {
        m[0] += rhs.m[0];
        m[1] += rhs.m[1];
        m[2] += rhs.m[2];

        m[3] += rhs.m[3];
        m[4] += rhs.m[4];
        m[5] += rhs.m[5];

        m[6] += rhs.m[6];
        m[7] += rhs.m[7];
        m[8] += rhs.m[8];
    }

    inline friend std::ostream& operator<<(std::ostream& out, const Matrix3x3& matrix3x3)
    {
        return out  << matrix3x3.ToString();
    }

    inline std::string ToString() const
    {
        return std::to_string(m[0]) + ", " + std::to_string(m[1]) + ", " + std::to_string(m[2]) + "\n" 
             + std::to_string(m[3]) + ", " + std::to_string(m[4]) + ", " + std::to_string(m[5]) + "\n" 
             + std::to_string(m[6]) + ", " + std::to_string(m[7]) + ", " + std::to_string(m[8]) + "\n";
    }

    static const Matrix3x3 ZeroMatrix;
    static const Matrix3x3 IdentityMatrix;

    float m[9];
};

class GOKNAR_API Matrix
{
public:
    Matrix()
    {
        // For warning fix......?
        m[0] = 0.f;
        for (int i = 0; i < 16; i++)
        {
            m[i] = 0;
        }
    }

    Matrix(float value)
    {
        // For warning fix......?
        m[0] = 0.f;
        for (int i = 0; i < 16; i++)
        {
            m[i] = value;
        }
    }

    Matrix(float val1, float val2, float val3, float val4,
           float val5, float val6, float val7, float val8, 
           float val9, float val10, float val11, float val12, 
           float val13, float val14, float val15, float val16)
    {
        m[0] = val1;
        m[1] = val2;
        m[2] = val3;
        m[3] = val4;

        m[4] = val5;
        m[5] = val6;
        m[6] = val7;
        m[7] = val8;

        m[8] = val9;
        m[9] = val10;
        m[10] = val11;
        m[11] = val12;

        m[12] = val13;
        m[13] = val14;
        m[14] = val15;
        m[15] = val16;  
    }

    Matrix(const float matrix[16])
    {
        // For warning fix......?
        m[0] = 0.f;
        for (int i = 0; i < 16; i++)
        {
            m[i] = matrix[i];
        }
    }

    Matrix(const Matrix& other)
    {
        // For warning fix......?
        m[0] = 0.f;
        if(this != &other)
        {
            for (int i = 0; i < 16; i++)
            {
                m[i] = other.m[i];
            }
        }
    }

    Matrix(const Matrix3x3& other)
    {
        m[0] = other.m[0];
        m[1] = other.m[1];
        m[2] = other.m[2];
        m[3] = 0.f;

        m[4] = other.m[3];
        m[5] = other.m[4];
        m[6] = other.m[5];
        m[7] = 0.f;

        m[8] = other.m[6];
        m[9] = other.m[7];
        m[10] = other.m[8];
        m[11] = 0.f;

        m[12] = 0.f;
        m[13] = 0.f;
        m[14] = 0.f;
        m[15] = 1.f;
    }

    ~Matrix()
    {

    }

    Vector3 GetAxisVector(int i) const
    {
        return Vector3(m[i], m[i + 4], m[i + 8]);
    }

    Vector3 GetAxisVector(Axis axis) const
    {
        switch (axis)
        {
        case Axis::X:
            return GetForwardVector();
            break;
        case Axis::Y:
            return GetLeftVector();
            break;
        case Axis::Z:
            return GetUpVector();
            break;
        default:
            break;
        }
        return Vector3::ZeroVector;
    }

    Vector3 GetForwardVector() const
    {
        return Vector3(m[0], m[4], m[8]);
    }

    Vector3 GetLeftVector() const
    {
        return Vector3(m[1], m[5], m[9]);
    }

    Vector3 GetUpVector() const
    {
        return Vector3(m[2], m[6], m[10]);
    }

    inline Vector3 GetTranslation() const
    {
        return Vector3
        {
            m[3],
            m[7],
            m[11]
        };
    }

    inline bool ContainsNanOrInf() const
    {
        return GoknarMath::IsNanOrInf(m[0]) ||
               GoknarMath::IsNanOrInf(m[1]) ||
               GoknarMath::IsNanOrInf(m[2]) ||
               GoknarMath::IsNanOrInf(m[3]) ||

               GoknarMath::IsNanOrInf(m[4]) ||
               GoknarMath::IsNanOrInf(m[5]) ||
               GoknarMath::IsNanOrInf(m[6]) ||
               GoknarMath::IsNanOrInf(m[7]) ||

               GoknarMath::IsNanOrInf(m[8]) ||
               GoknarMath::IsNanOrInf(m[9]) ||
               GoknarMath::IsNanOrInf(m[10]) ||
               GoknarMath::IsNanOrInf(m[11]) ||

               GoknarMath::IsNanOrInf(m[12]) ||
               GoknarMath::IsNanOrInf(m[13]) ||
               GoknarMath::IsNanOrInf(m[14]) ||
               GoknarMath::IsNanOrInf(m[15]);
    }

	inline static Matrix GetRotationMatrix(const Vector3& rotation)
	{
        //Matrix result(Matrix::IdentityMatrix);

        //if (rotation.x != 0.f)
        //{
        //    Matrix xRotationMatrix = Matrix::IdentityMatrix;
        //    float cosTheta = cos(rotation.x);
        //    float sinTheta = sin(rotation.x);
        //    xRotationMatrix.m[5] = cosTheta;
        //    xRotationMatrix.m[6] = -sinTheta;
        //    xRotationMatrix.m[9] = sinTheta;
        //    xRotationMatrix.m[10] = cosTheta;
        //    result *= xRotationMatrix;
        //}
        //if (rotation.y != 0.f)
        //{
        //    Matrix yRotationMatrix = Matrix::IdentityMatrix;
        //    float cosTheta = cos(rotation.y);
        //    float sinTheta = sin(rotation.y);
        //    yRotationMatrix.m[0] = cosTheta;
        //    yRotationMatrix.m[2] = sinTheta;
        //    yRotationMatrix.m[8] = -sinTheta;
        //    yRotationMatrix.m[10] = cosTheta;
        //    result *= yRotationMatrix;
        //}
        //if (rotation.z != 0.f)
        //{
        //    Matrix zRotationMatrix = Matrix::IdentityMatrix;
        //    float cosTheta = cos(rotation.z);
        //    float sinTheta = sin(rotation.z);
        //    zRotationMatrix.m[0] = cosTheta;
        //    zRotationMatrix.m[1] = -sinTheta;
        //    zRotationMatrix.m[4] = sinTheta;
        //    zRotationMatrix.m[5] = cosTheta;
        //    result *= zRotationMatrix;
        //}

        //return result;

        const float cosAlpha = cos(rotation.x);
        const float sinAlpha = sin(rotation.x);

        const float cosBeta = cos(rotation.y);
        const float sinBeta = sin(rotation.y);

        const float cosGamma = cos(rotation.z);
        const float sinGamma = sin(rotation.z);

		return Matrix(cosBeta * cosGamma,                                   cosBeta * sinGamma, -sinBeta,                                               0.f,
                      sinAlpha * sinBeta * cosGamma - cosAlpha * sinGamma,  sinAlpha * sinBeta * sinGamma + cosAlpha * cosGamma, sinAlpha * cosBeta,    0.f,
                      cosAlpha * sinBeta * cosGamma + sinAlpha * sinGamma,  cosAlpha * sinBeta * sinGamma - sinAlpha * cosGamma, cosAlpha * cosBeta,    0.f,
                      0.f,                                                  0.f,                                                 0.f,                   1.f);
	}

    inline static Matrix GetPositionMatrix(const Vector3& position)
    {
        return Matrix(  1.f, 0.f, 0.f, position.x,
                        0.f, 1.f, 0.f, position.y,
                        0.f, 0.f, 1.f, position.z,
                        0.f, 0.f, 0.f, 1.f);
    }

    inline static Matrix GetScalingMatrix(const Vector3& scaling)
    {
        return Matrix(  scaling.x, 0.f, 0.f, 0.f,
                        0.f, scaling.y, 0.f, 0.f,
                        0.f, 0.f, scaling.z, 0.f,
                        0.f, 0.f, 0.f, 1.f);
    }

    inline static Matrix GetTransformationMatrix(const Quaternion& rotation, const Vector3& position, const Vector3& scaling)
    {
        // Since OpenGL uses column-major matriced and Goknar does not
        // all matrix multiplications are done in reverse order
        return GetPositionMatrix(position) * rotation.GetMatrix() * GetScalingMatrix(scaling);
    }

	inline static Matrix GetRotationMatrixAboutAnAxis(Vector3 axis, float angle)
	{
		float cosAngle = cos(angle);
		float sinAngle = sin(angle);
		float oneMinusCosAngle = 1.f - cosAngle;

		return Matrix(cosAngle + axis.x * axis.x * oneMinusCosAngle,				axis.x * axis.y * oneMinusCosAngle - axis.z * sinAngle,		axis.x * axis.z * oneMinusCosAngle + axis.y * sinAngle,		0.f,
					  axis.y * axis.x * oneMinusCosAngle + axis.z * sinAngle,		cosAngle + axis.y * axis.y * oneMinusCosAngle,				axis.y * axis.z * oneMinusCosAngle - axis.x * sinAngle,		0.f,
					  axis.z * axis.x * oneMinusCosAngle - axis.y * sinAngle,		axis.z * axis.y * oneMinusCosAngle + axis.x * sinAngle,		cosAngle + axis.z * axis.z * oneMinusCosAngle,				0.f,
					  0.f,															0.f,														0.f,														1.f);
	}

    inline Vector4 MultiplyTransposeBy(const Vector4& vector) const
    {
        return Vector4( vector.x * m[0] + vector.y * m[3] + vector.z * m[6]+ vector.z * m[10],
                        vector.x * m[1] + vector.y * m[4] + vector.z * m[7] + vector.z * m[11],
                        vector.x * m[2] + vector.y * m[5] + vector.z * m[8] + vector.z * m[12],
                        vector.x * m[3] + vector.y * m[6] + vector.z * m[9] + vector.z * m[13]);
    }


    /**
     * From Cyclone Physics
     * 
     * Transform the given vector by the transformational inverse
     * of this matrix.
     *
     * @note This function relies on the fact that the inverse of
     * a pure rotation matrix is its transpose. It separates the
     * translational and rotation components, transposes the
     * rotation, and multiplies out. If the matrix is not a
     * scale and shear free transform matrix, then this function
     * will not give correct results.
     *
     * @param vector The vector to transform.
     */
    inline Vector3 MultiplyTransposeByInverse(const Vector3& vector) const
    {
        Vector3 tmp = vector;
        tmp.x -= m[3];
        tmp.y -= m[7];
        tmp.z -= m[11];
        return Vector3(
            tmp.x * m[0] +
            tmp.y * m[4] +
            tmp.z * m[8],

            tmp.x * m[1] +
            tmp.y * m[5] +
            tmp.z * m[9],

            tmp.x * m[2] +
            tmp.y * m[6] +
            tmp.z * m[10]
        );
    }

    /**
     * From Cyclone Physics
     * 
     * Transform the given direction vector by the
     * transformational inverse of this matrix.
     *
     * @note This function relies on the fact that the inverse of
     * a pure rotation matrix is its transpose. It separates the
     * translational and rotation components, transposes the
     * rotation, and multiplies out. If the matrix is not a
     * scale and shear free transform matrix, then this function
     * will not give correct results.
     *
     * @note When a direction is converted between frames of
     * reference, there is no translation required.
     *
     * @param vector The vector to transform.
     */
    Vector3 MultiplyTransposeByInverseDirection(const Vector3& vector) const
    {
        return Vector3(
            vector.x * m[0] +
            vector.y * m[4] +
            vector.z * m[8],

            vector.x * m[1] +
            vector.y * m[5] +
            vector.z * m[9],

            vector.x * m[2] +
            vector.y * m[6] +
            vector.z * m[10]
        );
    }

    void operator=(const Matrix& rhs)
    {
        if(this != &rhs)
        {
            for (int i = 0; i < 16; i++)
            {
                m[i] = rhs.m[i];
            }
        }
    }

	Matrix operator-()
	{
		Matrix result(*this);

		for (int i = 0; i < 16; i++)
		{
			result.m[i] *= -1;
		}

		return result;
	}

    Matrix operator*(float value) const
    {
        return Matrix(
            m[0] * value, m[1] * value, m[2] * value, m[3] * value,
            m[4] * value, m[5] * value, m[6] * value, m[7] * value,
            m[8] * value, m[9] * value, m[10] * value, m[11] * value,
            m[12] * value, m[13] * value, m[14] * value, m[15] * value
            );
    }

    Vector4 operator*(const Vector4& rhs) const
    {
        Vector4 out(0);

        out.x = m[0] * rhs.x + m[1] * rhs.y + m[2] * rhs.z + m[3] * rhs.w;
        out.y = m[4] * rhs.x + m[5] * rhs.y + m[6] * rhs.z + m[7] * rhs.w;
        out.z = m[8] * rhs.x + m[9] * rhs.y + m[10] * rhs.z + m[11] * rhs.w;
        out.w = m[12] * rhs.x + m[13] * rhs.y + m[14] * rhs.z + m[15] * rhs.w;

        return out;
    }

    //Quaternion operator*(const Quaternion& rhs) const
    //{
    //    Quaternion out;

    //    out.x = m[0] * rhs.x + m[1] * rhs.y + m[2] * rhs.z + m[3] * rhs.w;
    //    out.y = m[4] * rhs.x + m[5] * rhs.y + m[6] * rhs.z + m[7] * rhs.w;
    //    out.z = m[8] * rhs.x + m[9] * rhs.y + m[10] * rhs.z + m[11] * rhs.w;
    //    out.w = m[12] * rhs.x + m[13] * rhs.y + m[14] * rhs.z + m[15] * rhs.w;

    //    return out;
    //}

    Matrix operator*(const Matrix& rhs) const
    {
        Matrix out(0.f);

        out.m[0] = m[0] * rhs.m[0] + m[1] * rhs.m[4] + m[2] * rhs.m[8] + m[3] * rhs.m[12];
        out.m[1] = m[0] * rhs.m[1] + m[1] * rhs.m[5] + m[2] * rhs.m[9] + m[3] * rhs.m[13];
        out.m[2] = m[0] * rhs.m[2] + m[1] * rhs.m[6] + m[2] * rhs.m[10] + m[3] * rhs.m[14];
        out.m[3] = m[0] * rhs.m[3] + m[1] * rhs.m[7] + m[2] * rhs.m[11] + m[3] * rhs.m[15];

        out.m[4] = m[4] * rhs.m[0] + m[5] * rhs.m[4] + m[6] * rhs.m[8] + m[7] * rhs.m[12];
        out.m[5] = m[4] * rhs.m[1] + m[5] * rhs.m[5] + m[6] * rhs.m[9] + m[7] * rhs.m[13];
        out.m[6] = m[4] * rhs.m[2] + m[5] * rhs.m[6] + m[6] * rhs.m[10] + m[7] * rhs.m[14];
        out.m[7] = m[4] * rhs.m[3] + m[5] * rhs.m[7] + m[6] * rhs.m[11] + m[7] * rhs.m[15];

        out.m[8] = m[8] * rhs.m[0] + m[9] * rhs.m[4] + m[10] * rhs.m[8] + m[11] * rhs.m[12];
        out.m[9] = m[8] * rhs.m[1] + m[9] * rhs.m[5] + m[10] * rhs.m[9] + m[11] * rhs.m[13];
        out.m[10] = m[8] * rhs.m[2] + m[9] * rhs.m[6] + m[10] * rhs.m[10] + m[11] * rhs.m[14];
        out.m[11] = m[8] * rhs.m[3] + m[9] * rhs.m[7] + m[10] * rhs.m[11] + m[11] * rhs.m[15];

        out.m[12] = m[12] * rhs.m[0] + m[13] * rhs.m[4] + m[14] * rhs.m[8] + m[15] * rhs.m[12];
        out.m[13] = m[12] * rhs.m[1] + m[13] * rhs.m[5] + m[14] * rhs.m[9] + m[15] * rhs.m[13];
        out.m[14] = m[12] * rhs.m[2] + m[13] * rhs.m[6] + m[14] * rhs.m[10] + m[15] * rhs.m[14];
        out.m[15] = m[12] * rhs.m[3] + m[13] * rhs.m[7] + m[14] * rhs.m[11] + m[15] * rhs.m[15];

        return out;
    }

	inline void operator*=(const Matrix& rhs)
	{
		Matrix out(0.f);

		out.m[0] = m[0] * rhs.m[0] + m[1] * rhs.m[4] + m[2] * rhs.m[8] + m[3] * rhs.m[12];
		out.m[1] = m[0] * rhs.m[1] + m[1] * rhs.m[5] + m[2] * rhs.m[9] + m[3] * rhs.m[13];
		out.m[2] = m[0] * rhs.m[2] + m[1] * rhs.m[6] + m[2] * rhs.m[10] + m[3] * rhs.m[14];
		out.m[3] = m[0] * rhs.m[3] + m[1] * rhs.m[7] + m[2] * rhs.m[11] + m[3] * rhs.m[15];

		out.m[4] = m[4] * rhs.m[0] + m[5] * rhs.m[4] + m[6] * rhs.m[8] + m[7] * rhs.m[12];
		out.m[5] = m[4] * rhs.m[1] + m[5] * rhs.m[5] + m[6] * rhs.m[9] + m[7] * rhs.m[13];
		out.m[6] = m[4] * rhs.m[2] + m[5] * rhs.m[6] + m[6] * rhs.m[10] + m[7] * rhs.m[14];
		out.m[7] = m[4] * rhs.m[3] + m[5] * rhs.m[7] + m[6] * rhs.m[11] + m[7] * rhs.m[15];

		out.m[8] = m[8] * rhs.m[0] + m[9] * rhs.m[4] + m[10] * rhs.m[8] + m[11] * rhs.m[12];
		out.m[9] = m[8] * rhs.m[1] + m[9] * rhs.m[5] + m[10] * rhs.m[9] + m[11] * rhs.m[13];
		out.m[10] = m[8] * rhs.m[2] + m[9] * rhs.m[6] + m[10] * rhs.m[10] + m[11] * rhs.m[14];
		out.m[11] = m[8] * rhs.m[3] + m[9] * rhs.m[7] + m[10] * rhs.m[11] + m[11] * rhs.m[15];

		out.m[12] = m[12] * rhs.m[0] + m[13] * rhs.m[4] + m[14] * rhs.m[8] + m[15] * rhs.m[12];
		out.m[13] = m[12] * rhs.m[1] + m[13] * rhs.m[5] + m[14] * rhs.m[9] + m[15] * rhs.m[13];
		out.m[14] = m[12] * rhs.m[2] + m[13] * rhs.m[6] + m[14] * rhs.m[10] + m[15] * rhs.m[14];
		out.m[15] = m[12] * rhs.m[3] + m[13] * rhs.m[7] + m[14] * rhs.m[11] + m[15] * rhs.m[15];

		*this = out;
	}

	float operator[](int index)
	{
		if (index >= 0 && index <= 15)
		{
			return m[index];
		}

        return 0.f;
	}

	const float operator[](int index) const
	{
		if (index >= 0 && index <= 15)
		{
			return m[index];
		}

        return 0.f;
	}

    bool operator==(const Matrix &rhs)
    {
        return Equals(rhs);
    }

    inline friend std::ostream& operator<<(std::ostream& out, const Matrix& matrix)
    {
        return out << matrix.ToString();
    }

    inline std::string ToString() const
    {
        return std::to_string(m[0]) + ", " + std::to_string(m[1]) + ", " + std::to_string(m[2]) + std::to_string(m[3]) + "\n"
             + std::to_string(m[4]) + ", " + std::to_string(m[5]) + ", " + std::to_string(m[6]) + std::to_string(m[7]) + "\n"
             + std::to_string(m[8]) + ", " + std::to_string(m[9]) + ", " + std::to_string(m[10]) + std::to_string(m[11]) + "\n"
             + std::to_string(m[12]) + ", " + std::to_string(m[13]) + ", " + std::to_string(m[14]) + std::to_string(m[15]) + "\n";
    }

    Matrix GetInverse() const;
    Matrix GetTranspose() const;
    Matrix3x3 GetUpper3x3() const;

    inline bool Equals(const Matrix& other, float tolerance = EPSILON) const
    {
        return
            std::abs(m[0] - other.m[0]) <= tolerance &&
            std::abs(m[1] - other.m[1]) <= tolerance &&
            std::abs(m[2] - other.m[2]) <= tolerance &&
            std::abs(m[3] - other.m[3]) <= tolerance &&
            std::abs(m[4] - other.m[4]) <= tolerance &&
            std::abs(m[5] - other.m[5]) <= tolerance &&
            std::abs(m[6] - other.m[6]) <= tolerance &&
            std::abs(m[7] - other.m[7]) <= tolerance &&
            std::abs(m[8] - other.m[8]) <= tolerance &&
            std::abs(m[9] - other.m[9]) <= tolerance &&
            std::abs(m[10] - other.m[10]) <= tolerance &&
            std::abs(m[11] - other.m[11]) <= tolerance &&
            std::abs(m[12] - other.m[12]) <= tolerance &&
            std::abs(m[13] - other.m[13]) <= tolerance &&
            std::abs(m[14] - other.m[14]) <= tolerance &&
            std::abs(m[15] - other.m[15]) <= tolerance;
    }

    static const Matrix ZeroMatrix;
    static const Matrix IdentityMatrix;

    float m[16];
};


#endif