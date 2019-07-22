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

protected:

private:
	unsigned int programId_;

};

#endif