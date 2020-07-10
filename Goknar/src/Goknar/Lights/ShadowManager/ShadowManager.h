#ifndef __SHADOWMANAGAR_H__
#define __SHADOWMANAGAR_H__

class Material;
class Shader;

class ShadowManager
{
public:
	ShadowManager();
	~ShadowManager();

	Material* GetDepthBufferMaterial() const
	{
		return depthBufferMaterial_;
	}

	void RenderShadowMaps();

protected:

private:
	Material* depthBufferMaterial_;
	Shader* depthBufferShader_;
};

#endif