#ifndef __SHADER_H__
#define __SHADER_H__

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

	void SetBool(const char* name, bool value);
	void SetInt(const char* name, int value) const;
	void SetFloat(const char* name, float value) const;
	void SetMatrix3x3(const char* name, const float* bufferPointer) const;
	void SetMatrix(const char* name, const float* bufferPointer) const;
	void SetVector3(const char* name, const float* bufferPointer) const;

protected:

private:
	unsigned int programId_;

};

#endif