#ifndef __SHADOWMANAGAR_H__
#define __SHADOWMANAGAR_H__

class Material;
class Shader;

class ShadowManager
{
public:
	ShadowManager() = default;
	~ShadowManager() = default;

	void RenderShadowMaps();

protected:

private:
};

#endif