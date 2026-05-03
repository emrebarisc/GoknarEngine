#ifndef __COMPUTESHADER_H__
#define __COMPUTESHADER_H__

#include "Types.h"

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/Matrix.h"

class GOKNAR_API ComputeShader
{
public:
	ComputeShader();
	~ComputeShader();

	const std::string& GetComputeShaderScript() const
	{
		return computeShaderScript_;
	}

	void SetComputeShaderScript(const std::string& computeShaderScript)
	{
		computeShaderScript_ = computeShaderScript;
	}

	void SetComputeShaderPath(const std::string& computeShaderPath)
	{
		computeShaderPath_ = ContentDir + computeShaderPath;
	}

	void SetComputeShaderPathAbsolute(const std::string& computeShaderPath)
	{
		computeShaderPath_ = computeShaderPath;
	}

	unsigned int GetProgramId() const
	{
		return programId_;
	}

	void PreInit();
	void Init();
	void PostInit();

	void Bind() const;
	void Unbind() const;
	void Use() const;

	void Dispatch(unsigned int groupCountX, unsigned int groupCountY = 1, unsigned int groupCountZ = 1) const;
	void Dispatch2D(unsigned int width, unsigned int height, unsigned int localSizeX = 8, unsigned int localSizeY = 8) const;

	void SetBool(const char* name, bool value) const;
	void SetInt(const char* name, int value) const;
	void SetFloat(const char* name, float value) const;
	void SetVector2(const char* name, const Vector2& vector) const;
	void SetMatrix(const char* name, const Matrix& matrix) const;
	void SetVector3(const char* name, const Vector3& vector) const;
	void SetVector4(const char* name, const Vector4& vector) const;

private:
	void UploadBool(const char* name, bool value) const;
	void UploadInt(const char* name, int value) const;
	void UploadFloat(const char* name, float value) const;
	void UploadVector2(const char* name, const Vector2& vector) const;
	void UploadMatrix(const char* name, const Matrix& matrix) const;
	void UploadVector3(const char* name, const Vector3& vector) const;
	void UploadVector4(const char* name, const Vector4& vector) const;

	std::string computeShaderPath_{ "" };
	std::string computeShaderScript_{ "" };

	GEuint programId_{ 0 };
};

#endif
