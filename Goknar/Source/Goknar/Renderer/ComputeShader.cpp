#include "pch.h"

#include "ComputeShader.h"

#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Graphics/IGraphicsAPI.h"
#include "Goknar/IO/IOManager.h"

namespace
{
	void ExitOnComputeShaderIsNotCompiled(GEuint shaderId, const char* errorMessage)
	{
		IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
		if (!graphicsAPI->GetShaderCompileStatus(shaderId))
		{
			const std::string logMessage = graphicsAPI->GetShaderInfoLog(shaderId);
			graphicsAPI->DeleteShader(shaderId);

			GOKNAR_CORE_ASSERT(false, "%s\nWhat went wrong: \n%s", errorMessage, logMessage.c_str());
		}
	}

	void ExitOnComputeProgramError(GEuint programId, const char* errorMessage)
	{
		IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
		if (!graphicsAPI->GetProgramLinkStatus(programId))
		{
			const std::string logMessage = graphicsAPI->GetProgramInfoLog(programId);
			graphicsAPI->DeleteProgram(programId);

			GOKNAR_CORE_ERROR("%s", logMessage.c_str());
			GOKNAR_CORE_ASSERT(false, errorMessage);
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
		engine->GetGraphicsAPI()->DeleteProgram(programId_);
		programId_ = 0;
	}
}

void ComputeShader::PreInit()
{
	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	programId_ = graphicsAPI->CreateProgram();

	GOKNAR_CORE_ASSERT(!computeShaderPath_.empty() || !computeShaderScript_.empty(), "No data to compile the compute shader! (No shader path or script is given.)");

	if (!computeShaderPath_.empty())
	{
		IOManager::ReadFile(computeShaderPath_.c_str(), computeShaderScript_);
	}

	const GEchar* computeSource = (const GEchar*)computeShaderScript_.c_str();
	GEuint computeShaderId = graphicsAPI->CreateShader(GraphicsShaderStage::Compute);
	graphicsAPI->SetShaderSource(computeShaderId, 1, &computeSource);
	graphicsAPI->CompileShader(computeShaderId);
	ExitOnComputeShaderIsNotCompiled(computeShaderId, (std::string("Compute shader compilation error!(") + computeShaderPath_ + ").").c_str());

	graphicsAPI->AttachShader(programId_, computeShaderId);
	graphicsAPI->LinkProgram(programId_);
	ExitOnComputeProgramError(programId_, "Compute shader program link error!");

	graphicsAPI->DetachShader(programId_, computeShaderId);
	graphicsAPI->DeleteShader(computeShaderId);
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
	engine->GetGraphicsAPI()->UseProgram(programId_);
}

void ComputeShader::Unbind() const
{
	engine->GetGraphicsAPI()->UseProgram(0);
}

void ComputeShader::Use() const
{
	Bind();
}

void ComputeShader::Dispatch(unsigned int groupCountX, unsigned int groupCountY, unsigned int groupCountZ) const
{
	Use();
	engine->GetGraphicsAPI()->DispatchCompute(groupCountX, groupCountY, groupCountZ);
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
	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	const GEint uniformLocation = graphicsAPI->GetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	graphicsAPI->SetUniform1i(uniformLocation, (int)value);
}

void ComputeShader::UploadInt(const char* name, int value) const
{
	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	const GEint uniformLocation = graphicsAPI->GetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	graphicsAPI->SetUniform1i(uniformLocation, value);
}

void ComputeShader::UploadFloat(const char* name, float value) const
{
	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	const GEint uniformLocation = graphicsAPI->GetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	graphicsAPI->SetUniform1f(uniformLocation, value);
}

void ComputeShader::UploadVector2(const char* name, const Vector2& vector) const
{
	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	const GEint uniformLocation = graphicsAPI->GetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	graphicsAPI->SetUniform2fv(uniformLocation, 1, &vector.x);
}

void ComputeShader::UploadMatrix(const char* name, const Matrix& matrix) const
{
	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	const GEint uniformLocation = graphicsAPI->GetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	graphicsAPI->SetUniformMatrix4fv(uniformLocation, 1, false, &matrix.m[0]);
}

void ComputeShader::UploadVector3(const char* name, const Vector3& vector) const
{
	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	const GEint uniformLocation = graphicsAPI->GetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	graphicsAPI->SetUniform3fv(uniformLocation, 1, &vector.x);
}

void ComputeShader::UploadVector4(const char* name, const Vector4& vector) const
{
	IGraphicsAPI* graphicsAPI = engine->GetGraphicsAPI();
	const GEint uniformLocation = graphicsAPI->GetUniformLocation(programId_, name);
	if (uniformLocation < 0)
	{
		return;
	}

	graphicsAPI->SetUniform4fv(uniformLocation, 1, &vector.x);
}
