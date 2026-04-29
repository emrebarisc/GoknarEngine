#include "pch.h"

#include "Shader.h"

#include "Goknar/Application.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Camera.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Contents/Image.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "Goknar/IO/IOManager.h"
#include "Goknar/Math/Matrix.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Texture.h"

#include <algorithm>
#include <sstream>
#include <string>

#include <glad/glad.h>

namespace
{
	constexpr const char* MATERIAL_NODE_METADATA_PREFIX = "// GOKNAR_MATERIAL_NODE|";

	std::string Trim(const std::string& value)
	{
		const size_t first = value.find_first_not_of(" \t\r\n");
		if (first == std::string::npos)
		{
			return "";
		}

		const size_t last = value.find_last_not_of(" \t\r\n");
		return value.substr(first, last - first + 1);
	}

	std::vector<std::string> SplitString(const std::string& value, char delimiter)
	{
		std::vector<std::string> parts;
		std::stringstream stream(value);
		std::string part;
		while (std::getline(stream, part, delimiter))
		{
			parts.push_back(part);
		}
		return parts;
	}

	std::unordered_map<std::string, std::string> ParseMetadataFields(const std::string& metadataText)
	{
		std::unordered_map<std::string, std::string> fields;
		for (const std::string& token : SplitString(metadataText, '|'))
		{
			const size_t separatorIndex = token.find('=');
			if (separatorIndex == std::string::npos)
			{
				continue;
			}

			fields[Trim(token.substr(0, separatorIndex))] = Trim(token.substr(separatorIndex + 1));
		}

		return fields;
	}

	bool ParseFloatValue(const std::string& value, float& outValue)
	{
		std::stringstream stream(Trim(value));
		stream >> outValue;
		return !stream.fail();
	}

	bool ParseVector2Value(const std::string& value, Vector2& outValue)
	{
		const std::vector<std::string> components = SplitString(value, ',');
		if (components.size() != 2)
		{
			return false;
		}

		return ParseFloatValue(components[0], outValue.x) && ParseFloatValue(components[1], outValue.y);
	}

	bool ParseVector3Value(const std::string& value, Vector3& outValue)
	{
		const std::vector<std::string> components = SplitString(value, ',');
		if (components.size() != 3)
		{
			return false;
		}

		return
			ParseFloatValue(components[0], outValue.x) &&
			ParseFloatValue(components[1], outValue.y) &&
			ParseFloatValue(components[2], outValue.z);
	}

	bool ParseVector4Value(const std::string& value, Vector4& outValue)
	{
		const std::vector<std::string> components = SplitString(value, ',');
		if (components.size() != 4)
		{
			return false;
		}

		return
			ParseFloatValue(components[0], outValue.x) &&
			ParseFloatValue(components[1], outValue.y) &&
			ParseFloatValue(components[2], outValue.z) &&
			ParseFloatValue(components[3], outValue.w);
	}
}

void ExitOnShaderIsNotCompiled(GEuint shaderId, const char* errorMessage)
{
	GEint isCompiled = GL_FALSE;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GEint maxLength = 4096;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

		GEchar* logMessage = new GEchar[maxLength + (GEint)1];
		glGetShaderInfoLog(shaderId, maxLength, &maxLength, logMessage);
		logMessage[maxLength] = '\0';
		glDeleteShader(shaderId);

		GOKNAR_CORE_ASSERT(false, "%s\nWhat went wrong: \n%s", errorMessage, logMessage);

		delete[] logMessage;
	}
}

void ExitOnProgramError(GEuint programId, const char* errorMessage)
{
	GLint isLinked = GL_FALSE;
	glGetProgramiv(programId, GL_LINK_STATUS, &isLinked);

	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 4096;
		glGetShaderiv(programId, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar* logMessage = new GLchar[maxLength + (GEint)1];
		glGetProgramInfoLog(programId, maxLength, &maxLength, logMessage);
		logMessage[maxLength] = '\0';
		glDeleteProgram(programId);

		GOKNAR_CORE_ERROR("%s", logMessage);
		GOKNAR_CORE_ASSERT(false, errorMessage);

		delete[] logMessage;
	}
}

Shader::Shader()
{
	//engine->GetApplication()->GetMainScene()->AddShader(this);
}

Shader::~Shader()
{
	namedValues_.clear();
	vertexShaderPath_.clear();
	fragmentShaderPath_.clear();
	geometryShaderPath_.clear();
	vertexShaderScript_.clear();
	fragmentShaderScript_.clear();
	geometryShaderScript_.clear();

	if (programId_)
	{
		glDeleteProgram(programId_);
		programId_ = 0;
	}
	//engine->GetApplication()->GetMainScene()->RemoveShader(this);
}

void Shader::SetMVP(const Matrix& worldAndRelativeTransformationMatrix) const
{
	SetMatrix(SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX, worldAndRelativeTransformationMatrix);

	const Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();

	SetMatrix(SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX, activeCamera->GetViewProjectionMatrix());
	SetVector3(SHADER_VARIABLE_NAMES::POSITIONING::VIEW_POSITION, activeCamera->GetPosition());
}

void Shader::PreInit()
{
	programId_ = glCreateProgram();

	// TODO: Change custom shader creation
	if (shaderType_ == ShaderType::Dependent || shaderType_ == ShaderType::SelfContained)
	{
		GOKNAR_CORE_ASSERT(((!vertexShaderPath_.empty() && !fragmentShaderPath_.empty()) ||
			(!vertexShaderScript_.empty() && !fragmentShaderScript_.empty())
			), "No data to compile the shader! (No shader paths or scripts are given.");

		if (!vertexShaderPath_.empty())
		{
			IOManager::ReadFile(vertexShaderPath_.c_str(), vertexShaderScript_);
		}

		if (!fragmentShaderPath_.empty())
		{
			IOManager::ReadFile(fragmentShaderPath_.c_str(), fragmentShaderScript_);
		}

		if (!geometryShaderPath_.empty())
		{
			IOManager::ReadFile(geometryShaderPath_.c_str(), geometryShaderScript_);
		}
	}
	//////////////////////////////////////

	ParseStoredValuesFromShaderScripts();

	const GEchar* vertexSource = (const GEchar*)vertexShaderScript_.c_str();
	GEuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderId, 1, &vertexSource, 0);
	glCompileShader(vertexShaderId);
	ExitOnShaderIsNotCompiled(vertexShaderId, (std::string("Vertex shader compilation error!(" + vertexShaderPath_ + ").")).c_str());

	glAttachShader(programId_, vertexShaderId);

	const GEchar* fragmentSource = (const GEchar*)fragmentShaderScript_.c_str();
	GEuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderId, 1, &fragmentSource, 0);
	glCompileShader(fragmentShaderId);
	ExitOnShaderIsNotCompiled(fragmentShaderId, (std::string("Fragment shader compilation error!(") + fragmentShaderPath_ + ").").c_str());

	glAttachShader(programId_, fragmentShaderId);

	GEuint geometryShaderId = 0;
	bool containsGeometryShader = !geometryShaderScript_.empty();
	if (containsGeometryShader)
	{
		const GEchar* geometrySource = (const GEchar*)geometryShaderScript_.c_str();
		geometryShaderId = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShaderId, 1, &geometrySource, 0);
		glCompileShader(geometryShaderId);
		ExitOnShaderIsNotCompiled(geometryShaderId, (std::string("Geometry shader compilation error!(") + geometryShaderPath_ + ").").c_str());

		glAttachShader(programId_, geometryShaderId);
	}

	glLinkProgram(programId_);
	ExitOnProgramError(programId_, "Shader program link error!");

	glDetachShader(programId_, vertexShaderId);
	glDetachShader(programId_, fragmentShaderId);
	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);
	if (containsGeometryShader)
	{
		glDetachShader(programId_, geometryShaderId);
		glDeleteShader(geometryShaderId);
	}
}

void Shader::Init()
{
	Bind();

	int textureSize = (int)textures_.size();
	for (int textureIndex = 0; textureIndex < textureSize; textureIndex++)
	{
		textures_[textureIndex]->Bind(this);
	}

	ExitOnProgramError(programId_, "Shader error on binding texture!");

	Unbind();
}

void Shader::PostInit()
{
	vertexShaderScript_.clear();
	fragmentShaderScript_.clear();
	geometryShaderScript_.clear();
}

void Shader::Bind() const
{
	glUseProgram(programId_);
}

void Shader::Unbind() const
{
	glUseProgram(0);
}

void Shader::Use() const
{
	Bind();
	ApplyStoredValuesIfNeeded();
}

void Shader::SetBool(const char* name, bool value) const
{
	StoreNamedValue(name, value);
	if (!programId_)
	{
		storedValuesNeedUpload_ = true;
		return;
	}

	Use();
	UploadBool(name, value);
}

void Shader::SetInt(const char* name, int value) const
{
	StoreNamedValue(name, value);
	if (!programId_)
	{
		storedValuesNeedUpload_ = true;
		return;
	}

	Use();
	UploadInt(name, value);
}

void Shader::SetIntVector(const char* name, const std::vector<int>& values) const
{
	if (values.empty())
	{
		return;
	}

	StoreNamedValue(name, values);
	if (!programId_)
	{
		storedValuesNeedUpload_ = true;
		return;
	}

	Use();
	UploadIntVector(name, values);
}

void Shader::SetFloat(const char* name, float value) const
{
	StoreNamedValue(name, value);
	if (!programId_)
	{
		storedValuesNeedUpload_ = true;
		return;
	}

	Use();
	UploadFloat(name, value);
}

void Shader::SetVector2(const char* name, const Vector2& vector) const
{
	StoreNamedValue(name, vector);
	if (!programId_)
	{
		storedValuesNeedUpload_ = true;
		return;
	}

	Use();
	UploadVector2(name, vector);
}

void Shader::SetMatrix(const char* name, const Matrix& matrix) const
{
	StoreNamedValue(name, matrix);
	if (!programId_)
	{
		storedValuesNeedUpload_ = true;
		return;
	}

	Use();
	UploadMatrix(name, matrix);
}

void Shader::SetMatrixVector(const char* name, const std::vector<Matrix>& matrixVector) const
{
	if (matrixVector.empty())
	{
		return;
	}

	StoreNamedValue(name, matrixVector);
	if (!programId_)
	{
		storedValuesNeedUpload_ = true;
		return;
	}

	Use();
	UploadMatrixVector(name, matrixVector);
}

void Shader::SetMatrixArray(const char* name, const Matrix* matrixArray, int size) const
{
	if (!matrixArray || size <= 0)
	{
		return;
	}

	SetMatrixVector(name, std::vector<Matrix>(matrixArray, matrixArray + size));
}

void Shader::SetVector3(const char* name, const Vector3& vector) const
{
	StoreNamedValue(name, vector);
	if (!programId_)
	{
		storedValuesNeedUpload_ = true;
		return;
	}

	Use();
	UploadVector3(name, vector);
}

void Shader::SetVector4(const char* name, const Vector4& vector) const
{
	StoreNamedValue(name, vector);
	if (!programId_)
	{
		storedValuesNeedUpload_ = true;
		return;
	}

	Use();
	UploadVector4(name, vector);
}

void Shader::SetArrayOfFloat(const char* name, const std::vector<float>& values) const
{
	if (values.empty())
	{
		return;
	}

	StoreNamedValue(name, values);
	if (!programId_)
	{
		storedValuesNeedUpload_ = true;
		return;
	}

	Use();
	UploadArrayOfFloat(name, values);
}

void Shader::SetArrayOfVector2(const char* name, const std::vector<Vector2>& values) const
{
	if (values.empty())
	{
		return;
	}

	StoreNamedValue(name, values);
	if (!programId_)
	{
		storedValuesNeedUpload_ = true;
		return;
	}

	Use();
	UploadArrayOfVector2(name, values);
}

void Shader::SetArrayOfVector3(const char* name, const std::vector<Vector3>& values) const
{
	if (values.empty())
	{
		return;
	}

	StoreNamedValue(name, values);
	if (!programId_)
	{
		storedValuesNeedUpload_ = true;
		return;
	}

	Use();
	UploadArrayOfVector3(name, values);
}

void Shader::SetArrayOfVector4(const char* name, const std::vector<Vector4>& values) const
{
	if (values.empty())
	{
		return;
	}

	StoreNamedValue(name, values);
	if (!programId_)
	{
		storedValuesNeedUpload_ = true;
		return;
	}

	Use();
	UploadArrayOfVector4(name, values);
}

bool Shader::GetFloat(const char* name, float& outValue) const
{
	NamedShaderValue namedValue;
	if (!TryGetNamedValue(name, namedValue))
	{
		return false;
	}

	if (const float* value = std::get_if<float>(&namedValue))
	{
		outValue = *value;
		return true;
	}

	return false;
}

bool Shader::GetVector2(const char* name, Vector2& outValue) const
{
	NamedShaderValue namedValue;
	if (!TryGetNamedValue(name, namedValue))
	{
		return false;
	}

	if (const Vector2* value = std::get_if<Vector2>(&namedValue))
	{
		outValue = *value;
		return true;
	}

	return false;
}

bool Shader::GetVector3(const char* name, Vector3& outValue) const
{
	NamedShaderValue namedValue;
	if (!TryGetNamedValue(name, namedValue))
	{
		return false;
	}

	if (const Vector3* value = std::get_if<Vector3>(&namedValue))
	{
		outValue = *value;
		return true;
	}

	return false;
}

bool Shader::GetVector4(const char* name, Vector4& outValue) const
{
	NamedShaderValue namedValue;
	if (!TryGetNamedValue(name, namedValue))
	{
		return false;
	}

	if (const Vector4* value = std::get_if<Vector4>(&namedValue))
	{
		outValue = *value;
		return true;
	}

	return false;
}

bool Shader::GetArrayOfFloat(const char* name, std::vector<float>& outValues) const
{
	NamedShaderValue namedValue;
	if (!TryGetNamedValue(name, namedValue))
	{
		return false;
	}

	if (const std::vector<float>* value = std::get_if<std::vector<float>>(&namedValue))
	{
		outValues = *value;
		return true;
	}

	return false;
}

bool Shader::GetArrayOfVector2(const char* name, std::vector<Vector2>& outValues) const
{
	NamedShaderValue namedValue;
	if (!TryGetNamedValue(name, namedValue))
	{
		return false;
	}

	if (const std::vector<Vector2>* value = std::get_if<std::vector<Vector2>>(&namedValue))
	{
		outValues = *value;
		return true;
	}

	return false;
}

bool Shader::GetArrayOfVector3(const char* name, std::vector<Vector3>& outValues) const
{
	NamedShaderValue namedValue;
	if (!TryGetNamedValue(name, namedValue))
	{
		return false;
	}

	if (const std::vector<Vector3>* value = std::get_if<std::vector<Vector3>>(&namedValue))
	{
		outValues = *value;
		return true;
	}

	return false;
}

bool Shader::GetArrayOfVector4(const char* name, std::vector<Vector4>& outValues) const
{
	NamedShaderValue namedValue;
	if (!TryGetNamedValue(name, namedValue))
	{
		return false;
	}

	if (const std::vector<Vector4>* value = std::get_if<std::vector<Vector4>>(&namedValue))
	{
		outValues = *value;
		return true;
	}

	return false;
}

void Shader::ParseStoredValuesFromShaderScripts()
{
	ParseStoredValuesFromShaderScript(vertexShaderScript_);
	ParseStoredValuesFromShaderScript(fragmentShaderScript_);
	storedValuesNeedUpload_ = !namedValues_.empty();
}

void Shader::ParseStoredValuesFromShaderScript(const std::string& shaderScript)
{
	std::stringstream stream(shaderScript);
	std::string line;
	while (std::getline(stream, line))
	{
		const size_t markerIndex = line.find(MATERIAL_NODE_METADATA_PREFIX);
		if (markerIndex == std::string::npos)
		{
			continue;
		}

		const std::unordered_map<std::string, std::string> fields =
			ParseMetadataFields(line.substr(markerIndex + std::char_traits<char>::length(MATERIAL_NODE_METADATA_PREFIX)));

		auto nameIterator = fields.find("name");
		auto typeIterator = fields.find("type");
		auto storageIterator = fields.find("storage");
		auto valuesIterator = fields.find("values");
		if (nameIterator == fields.end() ||
			typeIterator == fields.end() ||
			storageIterator == fields.end() ||
			valuesIterator == fields.end() ||
			storageIterator->second != "Uniform" ||
			namedValues_.find(nameIterator->second) != namedValues_.end())
		{
			continue;
		}

		const std::string& name = nameIterator->second;
		const std::string& type = typeIterator->second;
		const bool isArray = fields.find("kind") != fields.end() && fields.at("kind") == "Array";

		if (isArray)
		{
			if (type == "Float")
			{
				std::vector<float> values;
				for (const std::string& entry : SplitString(valuesIterator->second, ';'))
				{
					float value = 0.0f;
					if (ParseFloatValue(entry, value))
					{
						values.push_back(value);
					}
				}
				if (!values.empty())
				{
					StoreNamedValue(name.c_str(), values);
				}
			}
			else if (type == "Vector2")
			{
				std::vector<Vector2> values;
				for (const std::string& entry : SplitString(valuesIterator->second, ';'))
				{
					Vector2 value(0.f);
					if (ParseVector2Value(entry, value))
					{
						values.push_back(value);
					}
				}
				if (!values.empty())
				{
					StoreNamedValue(name.c_str(), values);
				}
			}
			else if (type == "Vector3")
			{
				std::vector<Vector3> values;
				for (const std::string& entry : SplitString(valuesIterator->second, ';'))
				{
					Vector3 value(0.f);
					if (ParseVector3Value(entry, value))
					{
						values.push_back(value);
					}
				}
				if (!values.empty())
				{
					StoreNamedValue(name.c_str(), values);
				}
			}
			else if (type == "Vector4")
			{
				std::vector<Vector4> values;
				for (const std::string& entry : SplitString(valuesIterator->second, ';'))
				{
					Vector4 value(0.f);
					if (ParseVector4Value(entry, value))
					{
						values.push_back(value);
					}
				}
				if (!values.empty())
				{
					StoreNamedValue(name.c_str(), values);
				}
			}

			continue;
		}

		if (type == "Float")
		{
			float value = 0.0f;
			if (ParseFloatValue(valuesIterator->second, value))
			{
				StoreNamedValue(name.c_str(), value);
			}
		}
		else if (type == "Vector2")
		{
			Vector2 value(0.f);
			if (ParseVector2Value(valuesIterator->second, value))
			{
				StoreNamedValue(name.c_str(), value);
			}
		}
		else if (type == "Vector3")
		{
			Vector3 value(0.f);
			if (ParseVector3Value(valuesIterator->second, value))
			{
				StoreNamedValue(name.c_str(), value);
			}
		}
		else if (type == "Vector4")
		{
			Vector4 value(0.f);
			if (ParseVector4Value(valuesIterator->second, value))
			{
				StoreNamedValue(name.c_str(), value);
			}
		}
	}
}

void Shader::ApplyStoredValuesIfNeeded() const
{
	if (!storedValuesNeedUpload_)
	{
		return;
	}

	ApplyStoredValues();
}

void Shader::ApplyStoredValues() const
{
	if (!programId_ || namedValues_.empty())
	{
		storedValuesNeedUpload_ = false;
		return;
	}

	for (const auto& [name, value] : namedValues_)
	{
		std::visit([this, &name](const auto& storedValue)
			{
				using ValueType = std::decay_t<decltype(storedValue)>;
				if constexpr (std::is_same_v<ValueType, bool>)
				{
					UploadBool(name.c_str(), storedValue);
				}
				else if constexpr (std::is_same_v<ValueType, int>)
				{
					UploadInt(name.c_str(), storedValue);
				}
				else if constexpr (std::is_same_v<ValueType, float>)
				{
					UploadFloat(name.c_str(), storedValue);
				}
				else if constexpr (std::is_same_v<ValueType, Vector2>)
				{
					UploadVector2(name.c_str(), storedValue);
				}
				else if constexpr (std::is_same_v<ValueType, Vector3>)
				{
					UploadVector3(name.c_str(), storedValue);
				}
				else if constexpr (std::is_same_v<ValueType, Vector4>)
				{
					UploadVector4(name.c_str(), storedValue);
				}
				else if constexpr (std::is_same_v<ValueType, Matrix>)
				{
					UploadMatrix(name.c_str(), storedValue);
				}
				else if constexpr (std::is_same_v<ValueType, std::vector<int>>)
				{
					UploadIntVector(name.c_str(), storedValue);
				}
				else if constexpr (std::is_same_v<ValueType, std::vector<float>>)
				{
					UploadArrayOfFloat(name.c_str(), storedValue);
				}
				else if constexpr (std::is_same_v<ValueType, std::vector<Vector2>>)
				{
					UploadArrayOfVector2(name.c_str(), storedValue);
				}
				else if constexpr (std::is_same_v<ValueType, std::vector<Vector3>>)
				{
					UploadArrayOfVector3(name.c_str(), storedValue);
				}
				else if constexpr (std::is_same_v<ValueType, std::vector<Vector4>>)
				{
					UploadArrayOfVector4(name.c_str(), storedValue);
				}
				else if constexpr (std::is_same_v<ValueType, std::vector<Matrix>>)
				{
					UploadMatrixVector(name.c_str(), storedValue);
				}
			}, value);
	}

	storedValuesNeedUpload_ = false;
}

void Shader::StoreNamedValue(const char* name, const NamedShaderValue& value) const
{
	if (!name || !name[0])
	{
		return;
	}

	namedValues_[name] = value;
}

bool Shader::TryGetNamedValue(const char* name, NamedShaderValue& outValue) const
{
	if (!name || !name[0])
	{
		return false;
	}

	const auto iterator = namedValues_.find(name);
	if (iterator == namedValues_.end())
	{
		return false;
	}

	outValue = iterator->second;
	return true;
}

void Shader::UploadBool(const char* name, bool value) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform1i(uniformLocation, (int)value);
}

void Shader::UploadInt(const char* name, int value) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform1i(uniformLocation, value);
}

void Shader::UploadIntVector(const char* name, const std::vector<int>& values) const
{
	if (values.empty())
	{
		return;
	}

	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform1iv(uniformLocation, (GLsizei)values.size(), values.data());
}

void Shader::UploadFloat(const char* name, float value) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform1f(uniformLocation, value);
}

void Shader::UploadVector2(const char* name, const Vector2& vector) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform2fv(uniformLocation, 1, &vector.x);
}

void Shader::UploadMatrix(const char* name, const Matrix& matrix) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &matrix.m[0]);
}

void Shader::UploadMatrixVector(const char* name, const std::vector<Matrix>& matrixVector) const
{
	if (matrixVector.empty())
	{
		return;
	}

	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniformMatrix4fv(uniformLocation, (GLsizei)matrixVector.size(), GL_FALSE, &matrixVector[0].m[0]);
}

void Shader::UploadVector3(const char* name, const Vector3& vector) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform3fv(uniformLocation, 1, &vector.x);
}

void Shader::UploadVector4(const char* name, const Vector4& vector) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform4fv(uniformLocation, 1, &vector.x);
}

void Shader::UploadArrayOfFloat(const char* name, const std::vector<float>& values) const
{
	if (values.empty())
	{
		return;
	}

	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform1fv(uniformLocation, (GLsizei)values.size(), values.data());
}

void Shader::UploadArrayOfVector2(const char* name, const std::vector<Vector2>& values) const
{
	if (values.empty())
	{
		return;
	}

	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform2fv(uniformLocation, (GLsizei)values.size(), &values[0].x);
}

void Shader::UploadArrayOfVector3(const char* name, const std::vector<Vector3>& values) const
{
	if (values.empty())
	{
		return;
	}

	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform3fv(uniformLocation, (GLsizei)values.size(), &values[0].x);
}

void Shader::UploadArrayOfVector4(const char* name, const std::vector<Vector4>& values) const
{
	if (values.empty())
	{
		return;
	}

	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform4fv(uniformLocation, (GLsizei)values.size(), &values[0].x);
}
