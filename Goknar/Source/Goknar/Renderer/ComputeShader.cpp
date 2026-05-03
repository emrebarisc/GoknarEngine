#include "pch.h"

#include "ComputeShader.h"

#include "Goknar/GoknarAssert.h"
#include "Goknar/IO/IOManager.h"

#include <glad/glad.h>

namespace
{
	void ExitOnComputeShaderIsNotCompiled(GEuint shaderId, const char* errorMessage)
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

	void ExitOnComputeProgramError(GEuint programId, const char* errorMessage)
	{
		GLint isLinked = GL_FALSE;
		glGetProgramiv(programId, GL_LINK_STATUS, &isLinked);

		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 4096;
			glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &maxLength);

			GLchar* logMessage = new GLchar[maxLength + (GEint)1];
			glGetProgramInfoLog(programId, maxLength, &maxLength, logMessage);
			logMessage[maxLength] = '\0';
			glDeleteProgram(programId);

			GOKNAR_CORE_ERROR("%s", logMessage);
			GOKNAR_CORE_ASSERT(false, errorMessage);

			delete[] logMessage;
		}
	}
}

ComputeShader::ComputeShader()
{
}

ComputeShader::~ComputeShader()
{
	computeShaderPath_.clear();
	computeShaderScript_.clear();

	if (programId_)
	{
		glDeleteProgram(programId_);
		programId_ = 0;
	}
}

void ComputeShader::PreInit()
{
	programId_ = glCreateProgram();

	GOKNAR_CORE_ASSERT(!computeShaderPath_.empty() || !computeShaderScript_.empty(), "No data to compile the compute shader! (No shader path or script is given.)");

	if (!computeShaderPath_.empty())
	{
		IOManager::ReadFile(computeShaderPath_.c_str(), computeShaderScript_);
	}

	const GEchar* computeSource = (const GEchar*)computeShaderScript_.c_str();
	GEuint computeShaderId = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShaderId, 1, &computeSource, 0);
	glCompileShader(computeShaderId);
	ExitOnComputeShaderIsNotCompiled(computeShaderId, (std::string("Compute shader compilation error!(") + computeShaderPath_ + ").").c_str());

	glAttachShader(programId_, computeShaderId);
	glLinkProgram(programId_);
	ExitOnComputeProgramError(programId_, "Compute shader program link error!");

	glDetachShader(programId_, computeShaderId);
	glDeleteShader(computeShaderId);
}

void ComputeShader::Init()
{
}

void ComputeShader::PostInit()
{
	computeShaderScript_.clear();
}

void ComputeShader::Bind() const
{
	glUseProgram(programId_);
}

void ComputeShader::Unbind() const
{
	glUseProgram(0);
}

void ComputeShader::Use() const
{
	Bind();
}

void ComputeShader::Dispatch(unsigned int groupCountX, unsigned int groupCountY, unsigned int groupCountZ) const
{
	Use();
	glDispatchCompute(groupCountX, groupCountY, groupCountZ);
}

void ComputeShader::Dispatch2D(unsigned int width, unsigned int height, unsigned int localSizeX, unsigned int localSizeY) const
{
	const unsigned int groupCountX = (width + localSizeX - 1) / localSizeX;
	const unsigned int groupCountY = (height + localSizeY - 1) / localSizeY;
	Dispatch(groupCountX, groupCountY, 1);
}

void ComputeShader::SetBool(const char* name, bool value) const
{
	Use();
	UploadBool(name, value);
}

void ComputeShader::SetInt(const char* name, int value) const
{
	Use();
	UploadInt(name, value);
}

void ComputeShader::SetFloat(const char* name, float value) const
{
	Use();
	UploadFloat(name, value);
}

void ComputeShader::SetVector2(const char* name, const Vector2& vector) const
{
	Use();
	UploadVector2(name, vector);
}

void ComputeShader::SetMatrix(const char* name, const Matrix& matrix) const
{
	Use();
	UploadMatrix(name, matrix);
}

void ComputeShader::SetVector3(const char* name, const Vector3& vector) const
{
	Use();
	UploadVector3(name, vector);
}

void ComputeShader::SetVector4(const char* name, const Vector4& vector) const
{
	Use();
	UploadVector4(name, vector);
}

void ComputeShader::UploadBool(const char* name, bool value) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform1i(uniformLocation, (int)value);
}

void ComputeShader::UploadInt(const char* name, int value) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform1i(uniformLocation, value);
}

void ComputeShader::UploadFloat(const char* name, float value) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform1f(uniformLocation, value);
}

void ComputeShader::UploadVector2(const char* name, const Vector2& vector) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform2fv(uniformLocation, 1, &vector.x);
}

void ComputeShader::UploadMatrix(const char* name, const Matrix& matrix) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &matrix.m[0]);
}

void ComputeShader::UploadVector3(const char* name, const Vector3& vector) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform3fv(uniformLocation, 1, &vector.x);
}

void ComputeShader::UploadVector4(const char* name, const Vector4& vector) const
{
	const GEint uniformLocation = glGetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	glUniform4fv(uniformLocation, 1, &vector.x);
}
