#ifndef __SHADER_H__
#define __SHADER_H__

#include "Types.h"

#include "ShaderTypes.h"

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/Matrix.h"

#include <unordered_map>
#include <variant>

class IMaterialBase;
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
	virtual ~Shader();

	const std::string& GetVertexShaderScript() const
	{
		return vertexShaderScript_;
	}

	void SetVertexShaderScript(const std::string& vertexShaderScript)
	{
		vertexShaderScript_ = vertexShaderScript;
	}

	const std::string& GetFragmentShaderScript()
	{
		return fragmentShaderScript_;
	}

	void SetFragmentShaderScript(const std::string& fragmentShaderScript)
	{
		fragmentShaderScript_ = fragmentShaderScript;
	}

	void SetGeometryShaderScript(const std::string& geometryShaderScript)
	{
		geometryShaderScript_ = geometryShaderScript;
	}

	const std::string& GetGeometryShaderScript()
	{
		return geometryShaderScript_;
	}

	void SetVertexShaderPath(const std::string& vertexShaderPath)
	{
		vertexShaderPath_ = ContentDir + vertexShaderPath;
	}

	void SetFragmentShaderPath(const std::string& fragmentShaderPath)
	{
		fragmentShaderPath_ = ContentDir + fragmentShaderPath;
	}

	void SetGeometryShaderPath(const std::string& geometryShaderPath)
	{
		geometryShaderPath_ = ContentDir + geometryShaderPath;
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

	void SetMVP(const Matrix& worldAndRelativeTransformationMatrix) const;

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

	void SetBool(const char* name, bool value) const;
	void SetInt(const char* name, int value) const;
	void SetIntVector(const char* name, const std::vector<int>& values) const;
	void SetFloat(const char* name, float value) const;
	void SetVector2(const char* name, const Vector2& vector) const;
	void SetMatrix(const char* name, const Matrix& matrix) const;
	void SetMatrixVector(const char* name, const std::vector<Matrix>& matrixVector) const;
	void SetMatrixArray(const char* name, const Matrix* matrixArray, int size) const;
	void SetVector3(const char* name, const Vector3& vector) const;
	void SetVector4(const char* name, const Vector4& vector) const;
	void SetArrayOfFloat(const char* name, const std::vector<float>& values) const;
	void SetArrayOfVector2(const char* name, const std::vector<Vector2>& values) const;
	void SetArrayOfVector3(const char* name, const std::vector<Vector3>& values) const;
	void SetArrayOfVector4(const char* name, const std::vector<Vector4>& values) const;

	bool GetFloat(const char* name, float& outValue) const;
	bool GetVector2(const char* name, Vector2& outValue) const;
	bool GetVector3(const char* name, Vector3& outValue) const;
	bool GetVector4(const char* name, Vector4& outValue) const;
	bool GetArrayOfFloat(const char* name, std::vector<float>& outValues) const;
	bool GetArrayOfVector2(const char* name, std::vector<Vector2>& outValues) const;
	bool GetArrayOfVector3(const char* name, std::vector<Vector3>& outValues) const;
	bool GetArrayOfVector4(const char* name, std::vector<Vector4>& outValues) const;

protected:

private:
	using NamedShaderValue = std::variant<
		bool,
		int,
		float,
		Vector2,
		Vector3,
		Vector4,
		Matrix,
		std::vector<int>,
		std::vector<float>,
		std::vector<Vector2>,
		std::vector<Vector3>,
		std::vector<Vector4>,
		std::vector<Matrix>>;

	void ParseStoredValuesFromShaderScripts();
	void ParseStoredValuesFromShaderScript(const std::string& shaderScript);
	void ApplyStoredValuesIfNeeded() const;
	void ApplyStoredValues() const;
	void StoreNamedValue(const char* name, const NamedShaderValue& value) const;
	bool TryGetNamedValue(const char* name, NamedShaderValue& outValue) const;

	void UploadBool(const char* name, bool value) const;
	void UploadInt(const char* name, int value) const;
	void UploadIntVector(const char* name, const std::vector<int>& values) const;
	void UploadFloat(const char* name, float value) const;
	void UploadVector2(const char* name, const Vector2& vector) const;
	void UploadMatrix(const char* name, const Matrix& matrix) const;
	void UploadMatrixVector(const char* name, const std::vector<Matrix>& matrixVector) const;
	void UploadVector3(const char* name, const Vector3& vector) const;
	void UploadVector4(const char* name, const Vector4& vector) const;
	void UploadArrayOfFloat(const char* name, const std::vector<float>& values) const;
	void UploadArrayOfVector2(const char* name, const std::vector<Vector2>& values) const;
	void UploadArrayOfVector3(const char* name, const std::vector<Vector3>& values) const;
	void UploadArrayOfVector4(const char* name, const std::vector<Vector4>& values) const;

	std::vector<const Texture*> textures_;
	mutable std::unordered_map<std::string, NamedShaderValue> namedValues_{};
	mutable bool storedValuesNeedUpload_{ false };

	std::string vertexShaderPath_{ "" };
	std::string fragmentShaderPath_{ "" };
	std::string geometryShaderPath_{ "" };

	std::string vertexShaderScript_{ "" };
	std::string fragmentShaderScript_{ "" };
	std::string geometryShaderScript_{ "" };

	GEuint programId_{ 0 };

	ShaderType shaderType_{ ShaderType::Scene };
};

#endif
