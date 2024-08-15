#ifndef __POSTPROCESSING_H__
#define __POSTPROCESSING_H__

#include "Goknar/Core.h"

class Shader;
class StaticMesh;

class GOKNAR_API PostProcessingEffect
{
public:
	PostProcessingEffect();
	~PostProcessingEffect();

	void PreInit();
	void Init();
	void PostInit();

	void SetShader(Shader* shader)
	{
		shader_ = shader;
	}

	Shader* GetShader() const
	{
		return shader_;
	}

	StaticMesh* GetPostProcessingMesh() const
	{
		return postProcessingMesh_;
	}

	void Render();

protected:

private:
	Shader* shader_{ nullptr };

	StaticMesh* postProcessingMesh_;
};

#endif