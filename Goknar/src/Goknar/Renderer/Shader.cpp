#include "pch.h"

#include "Shader.h"

#include "ShaderBuilder.h"

#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "Goknar/IO/IOManager.h"
#include "Goknar/Log.h"
#include "Goknar/Matrix.h"
#include "Goknar/Renderer/Shader.h"
#include "Texture.h"


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

		GOKNAR_CORE_ERROR("{0}", logMessage);
		GOKNAR_ASSERT(false, errorMessage);

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
		GOKNAR_ASSERT(false, errorMessage);

		delete[] logMessage;
	}
}

Shader::Shader() :
	shaderType_(ShaderType::Scene),
	programId_(0)
{
}

Shader::~Shader()
{
	glDeleteProgram(programId_);
}

void Shader::SetMVP(const Matrix& model/*, const Matrix& view, const Matrix& projection*/) const
{
	const Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();

	SetMatrix(SHADER_VARIABLE_NAMES::POSITIONING::MODEL_MATRIX, model);
	SetMatrix(SHADER_VARIABLE_NAMES::POSITIONING::VIEW_MATRIX, activeCamera->GetViewingMatrix());
	SetMatrix(SHADER_VARIABLE_NAMES::POSITIONING::PROJECTION_MATRIX, activeCamera->GetProjectionMatrix());
	SetVector3(SHADER_VARIABLE_NAMES::POSITIONING::VIEW_POSITION, activeCamera->GetPosition());
}

void Shader::Init()
{
	if (shaderType_ == ShaderType::Dependent || shaderType_ == ShaderType::SelfContained)
	{
		GOKNAR_ASSERT((vertexShaderPath_ != "" && fragmentShaderPath_ != ""), "Shader vertex and/or fragment shader path(s) are not given.");

		IOManager::ReadFile(vertexShaderPath_.c_str(), vertexShaderScript_);
		IOManager::ReadFile(fragmentShaderPath_.c_str(), fragmentShaderScript_);
	}
	else
	{
		vertexShaderScript_ = ShaderBuilder::GetInstance()->GetDefaultSceneVertexShader();
		fragmentShaderScript_ = ShaderBuilder::GetInstance()->GetDefaultSceneFragmentShader();
	}

	const GEchar* vertexSource = (const GEchar*)vertexShaderScript_.c_str();
	GEuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderId, 1, &vertexSource, 0);
	glCompileShader(vertexShaderId);
	ExitOnShaderIsNotCompiled(vertexShaderId, "Vertex shader compilation error!");

	const GEchar* fragmentSource = (const GEchar*)fragmentShaderScript_.c_str();
	GEuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderId, 1, &fragmentSource, 0);
	glCompileShader(fragmentShaderId);
	ExitOnShaderIsNotCompiled(fragmentShaderId, "Fragment shader compilation error!");

	programId_ = glCreateProgram();

	glAttachShader(programId_, vertexShaderId);
	glAttachShader(programId_, fragmentShaderId);

	glLinkProgram(programId_);

	ExitOnProgramError(programId_, "Shader program link error!");

	Bind();

	int textureSize = (int)textures_.size();
	for (int textureIndex = 0; textureIndex < textureSize; textureIndex++)
	{
		textures_[textureIndex]->Bind(this);
	}

	Unbind();

	glDetachShader(programId_, vertexShaderId);
	glDetachShader(programId_, fragmentShaderId);
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

void Shader::SetBool(const char* name, bool value)
{
	glUniform1i(glGetUniformLocation(programId_, name), (int)value);
}

void Shader::SetInt(const char* name, int value) const
{
	GEint comolokko = glGetUniformLocation(programId_, name);
	glUniform1i(glGetUniformLocation(programId_, name), value);
}

void Shader::SetFloat(const char* name, float value) const
{
	glUniform1f(glGetUniformLocation(programId_, name), value);
}

void Shader::SetMatrix(const char* name, const Matrix& matrix) const
{
	glUniformMatrix4fv(glGetUniformLocation(programId_, name), 1, GL_FALSE, &matrix.m[0]);
}

void Shader::SetVector3(const char* name, const Vector3& vector) const
{
	glUniform3fv(glGetUniformLocation(programId_, name), 1, &vector.x);
}