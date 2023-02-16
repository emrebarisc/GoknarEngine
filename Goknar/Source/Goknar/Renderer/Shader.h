#ifndef __SHADER_H__
#define __SHADER_H__

#include "Types.h"

#include "ShaderTypes.h"

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"

class Material;
class Texture;

enum class ShaderType
{
	// Undependent from any scene constraint(Not linked to lights etc.)
	SelfContained = 0,
	// Dependent to scene constraint(Linked to lights etc.)
	Dependent,
	// Use only scene shader built by the ShaderBuilder
	Scene
};

class GOKNAR_API Shader
{
public:
	Shader();
	~Shader();

	void SetVertexShaderScript(const std::string& vertexShaderScript)
	{
		vertexShaderScript_ = vertexShaderScript;
	}

	void SetFragmentShaderScript(const std::string& fragmentShaderScript)
	{
		fragmentShaderScript_ = fragmentShaderScript;
	}

	void SetVertexShaderPath(const std::string& vertexShaderPath)
	{
		vertexShaderPath_ = ContentDir + vertexShaderPath;
	}

	void SetFragmentShaderPath(const std::string& fragmentShaderPath)
	{
		fragmentShaderPath_ = ContentDir + fragmentShaderPath;
	}

	ShaderType GetShaderType() const
	{
		return shaderType_;
	}

	void SetShaderType(ShaderType shaderType)
	{
		shaderType_ = shaderType;
	}

	void AddTexture(const Texture* texture)
	{
		textures_.push_back(texture);
	}

	const std::vector<const Texture*>* GetTextures() const
	{
		return &textures_;
	}

	void SetMVP(const Matrix& worldTransformationMatrix, const Matrix& relativeTransformationMatrix) const;

	unsigned int GetProgramId() const
	{
		return programId_;
	}

	void Init();

	void Bind() const;

	void Unbind() const;

	void Use() const;

	void SetBool(const char* name, bool value);
	void SetInt(const char* name, int value) const;
	void SetFloat(const char* name, float value) const;
	void SetMatrix(const char* name, const Matrix& matrix) const;
	void SetMatrixVector(const char* name, const std::vector<Matrix>& matrixVector);
	void SetVector3(const char* name, const Vector3& vector) const;

protected:

private:
	std::vector<const Texture*> textures_;

	std::string vertexShaderPath_;
	std::string fragmentShaderPath_;

	std::string vertexShaderScript_;
	std::string fragmentShaderScript_;

	GEuint programId_;

	ShaderType shaderType_;
};

#endif