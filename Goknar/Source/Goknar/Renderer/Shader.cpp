#include "pch.h"

#include "Shader.h"

#include "ShaderBuilder.h"

#include "Goknar/Application.h"
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

#include <string>

#include <glad/glad.h>

void ExitOnShaderIsNotCompiled(GEuint shaderId, const char* errorMessage)
{
	GEint isCompiled = 0;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GEint maxLength = 0;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

		GEchar* logMessage = new GEchar[maxLength + (GEint)1];
		glGetShaderInfoLog(shaderId, maxLength, &maxLength, logMessage);
		logMessage[maxLength] = '\0';
		glDeleteShader(shaderId);

		GOKNAR_CORE_ASSERT(false, "{}\n What went wrong: {}", errorMessage, logMessage);

		delete[] logMessage;
	}
}

void ExitOnProgramError(GEuint programId, const char* errorMessage)
{
	GLint isLinked = 0;
	glGetProgramiv(programId, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(programId, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar* logMessage = new GLchar[maxLength + (GEint)1];
		glGetProgramInfoLog(programId, maxLength, &maxLength, logMessage);
		logMessage[maxLength] = '\0';
		glDeleteProgram(programId);

		GOKNAR_CORE_ERROR("{0}", logMessage);
		GOKNAR_CORE_ASSERT(false, errorMessage);

		delete[] logMessage;
	}
}

Shader::Shader()
{
	engine->GetApplication()->GetMainScene()->AddShader(this);
}

Shader::~Shader()
{
	glDeleteProgram(programId_);
	engine->GetApplication()->GetMainScene()->RemoveShader(this);
}

void Shader::SetMVP(const Matrix& worldTransformationMatrix, const Matrix& relativeTransformationMatrix/*, const Matrix& view, const Matrix& projection*/) const
{
	/* TODO: DECIDE WHETHER WORLD AND RELATIVE TRANSFORMATION MATRICES SHOULD BE MULTIPLIED AND SENT TO THE GPU OR THEY SHOULD BE SENT SEPERATELY */
	/* THIS MAY AFFECT PERFORMANCE ****************************************************************************************************************/
	/**/SetMatrix(SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX, worldTransformationMatrix * relativeTransformationMatrix);
	/**/SetMatrix(SHADER_VARIABLE_NAMES::POSITIONING::WORLD_TRANSFORMATION_MATRIX, worldTransformationMatrix);
	/**/SetMatrix(SHADER_VARIABLE_NAMES::POSITIONING::RELATIVE_TRANSFORMATION_MATRIX, relativeTransformationMatrix);
	/**********************************************************************************************************************************************/

	const Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();

	//SetMatrix(SHADER_VARIABLE_NAMES::POSITIONING::VIEW_MATRIX, activeCamera->GetViewingMatrix());
	//SetMatrix(SHADER_VARIABLE_NAMES::POSITIONING::PROJECTION_MATRIX, activeCamera->GetProjectionMatrix());
	SetMatrix(SHADER_VARIABLE_NAMES::POSITIONING::VIEW_PROJECTION_MATRIX, activeCamera->GetViewProjectionMatrix());
	SetVector3(SHADER_VARIABLE_NAMES::POSITIONING::VIEW_POSITION, activeCamera->GetPosition());
}

void Shader::PreInit()
{
}

void Shader::Init()
{
	programId_ = glCreateProgram();

	// TODO: Change custom shader creation
	if (shaderType_ == ShaderType::Dependent || shaderType_ == ShaderType::SelfContained)
	{
		GOKNAR_CORE_ASSERT(	(	(!vertexShaderPath_.empty() && !fragmentShaderPath_.empty()) ||
								(!vertexShaderScript_.empty() && !fragmentShaderScript_.empty())
					  		), "No data to compile the shader! (No shader paths or scripts are given.");

		if(!vertexShaderPath_.empty())
		{
			IOManager::ReadFile(vertexShaderPath_.c_str(), vertexShaderScript_);
		}

		if(!fragmentShaderPath_.empty())
		{
			IOManager::ReadFile(fragmentShaderPath_.c_str(), fragmentShaderScript_);
		}

		if(!geometryShaderPath_.empty())
		{
			IOManager::ReadFile(geometryShaderPath_.c_str(), geometryShaderScript_);
		}
	}
	//////////////////////////////////////

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

	Bind();

	int textureSize = (int)textures_.size();
	for (int textureIndex = 0; textureIndex < textureSize; textureIndex++)
	{
		textures_[textureIndex]->Bind(this);
	}

	engine->GetRenderer()->BindShadowTextures(this);
	ExitOnProgramError(programId_, "Shader error on binding texture!");

	Unbind();

	glDetachShader(programId_, vertexShaderId);
	glDetachShader(programId_, fragmentShaderId);
	if (containsGeometryShader)
	{
		glDetachShader(programId_, geometryShaderId);
	}
}

void Shader::PostInit()
{
	vertexShaderScript_.clear();
	fragmentShaderScript_.clear();
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
}

void Shader::SetBool(const char* name, bool value) const
{
	GEint uniformLocation = glGetUniformLocation(programId_, name);
	glUniform1i(uniformLocation, (int)value);
}

void Shader::SetInt(const char* name, int value) const
{
	GEint uniformLocation = glGetUniformLocation(programId_, name);
	glUniform1i(uniformLocation, value);
}

void Shader::SetFloat(const char* name, float value) const
{
	GEint uniformLocation = glGetUniformLocation(programId_, name);
	glUniform1f(uniformLocation, value);
}

void Shader::SetMatrix(const char* name, const Matrix& matrix) const
{
	GEint uniformLocation = glGetUniformLocation(programId_, name);
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &matrix.m[0]);
}

void Shader::SetMatrixVector(const char* name, const std::vector<Matrix>& matrixVector) const
{
	GEint uniformLocation = glGetUniformLocation(programId_, name);
	glUniformMatrix4fv(uniformLocation, matrixVector.size(), GL_FALSE, &matrixVector[0].m[0]);
}

void Shader::SetVector3(const char* name, const Vector3& vector) const
{
	GEint uniformLocation = glGetUniformLocation(programId_, name);
	glUniform3fv(uniformLocation, 1, &vector.x);
}