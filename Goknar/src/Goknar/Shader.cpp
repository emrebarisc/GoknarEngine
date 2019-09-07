#include "pch.h"

#include "Matrix.h"
#include "Shader.h"
#include "Log.h"

#include <glad/glad.h>

void ExitOnShaderIsNotCompiled(GLuint shaderId, const char* errorMessage)
{
	GLint isCompiled = 0;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar* logMessage = new GLchar[maxLength + 1];
		glGetShaderInfoLog(shaderId, maxLength, &maxLength, logMessage);
		logMessage[maxLength] = '\0';
		glDeleteShader(shaderId);

		GOKNAR_CORE_ERROR("{0}", logMessage);
		GOKNAR_ASSERT(false, errorMessage);

		delete[] logMessage;
	}
}

void ExitOnProgramError(GLuint programId, const char* errorMessage)
{
	GLint isLinked = 0;
	glGetProgramiv(programId, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(programId, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar* logMessage = new GLchar[maxLength + 1];
		glGetProgramInfoLog(programId, maxLength, &maxLength, logMessage);
		logMessage[maxLength] = '\0';
		glDeleteProgram(programId);

		GOKNAR_CORE_ERROR("{0}", logMessage);

		delete[] logMessage;
	}
}

Shader::Shader()
{

}

Shader::Shader(const char* vertexShaderSource, const char* fragmentShaderSource)
{
	const GLchar* vertexSource = (const GLchar*)vertexShaderSource;
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderId, 1, &vertexSource, 0);
	glCompileShader(vertexShaderId);
	ExitOnShaderIsNotCompiled(vertexShaderId, "Vertex shader compilation error!");

	const GLchar* fragmentSource = (const GLchar*)fragmentShaderSource;
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderId, 1, &fragmentSource, 0);
	glCompileShader(fragmentShaderId);
	ExitOnShaderIsNotCompiled(fragmentShaderId, "Fragment shader compilation error!");

	programId_ = glCreateProgram();

	glAttachShader(programId_, vertexShaderId);
	glAttachShader(programId_, fragmentShaderId);

	glLinkProgram(programId_);

	ExitOnProgramError(programId_, "Shader program link error!");

	glDetachShader(programId_, vertexShaderId);
	glDetachShader(programId_, fragmentShaderId);
}

Shader::~Shader()
{
	glDeleteProgram(programId_);
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
	glUseProgram(programId_);
}

void Shader::SetBool(const char* name, bool value)
{
	glUniform1i(glGetUniformLocation(programId_, name), (int)value);
}

void Shader::SetInt(const char* name, int value) const
{
	glUniform1i(glGetUniformLocation(programId_, name), value);
}

void Shader::SetFloat(const char* name, float value) const
{
	glUniform1f(glGetUniformLocation(programId_, name), value);
}

void Shader::SetMatrix(const char* name, const Matrix& matrix) const
{
	glUniformMatrix4fv(glGetUniformLocation(programId_, name), 1, GL_FALSE, &matrix[0]);
}

void Shader::SetVector3(const char* name, const Vector3& vector) const
{
	glUniform3fv(glGetUniformLocation(programId_, name), 1, &vector.x);
}