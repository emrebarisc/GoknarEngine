#ifndef __SHADER_H__
#define __SHADER_H__

#include "Types.h"

#include "Goknar/Math.h"

class Shader
{
public:
	Shader();
	Shader(const char* vertexShader, const char* fragmentShader);
	~Shader();

	void Bind() const;

	void Unbind() const;

	unsigned int GetProgramId() const
	{
		return programId_;
	}

	void Use() const;

	void SetBool(const char* name, bool value);
	void SetInt(const char* name, int value) const;
	void SetFloat(const char* name, float value) const;
	void SetMatrix(const char* name, const Matrix& matrix) const;
	void SetVector3(const char* name, const Vector3& vector) const;

protected:

private:
	GEuint programId_;

};

#endif