#ifndef __SHADOWMANAGAR_H__
#define __SHADOWMANAGAR_H__

class Material;
class Light;
class Shader;

class ShadowManager
{
public:
	ShadowManager() = default;
	~ShadowManager() = default;

	void RenderShadowMaps();
	void SetShadowRenderPassShaderUniforms(const Shader* shader) const;

protected:

private:

	Light* currentlyRenderingLight_;
};

#endif