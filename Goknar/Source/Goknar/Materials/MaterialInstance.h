#ifndef __MATERIALINSTANCE_H__
#define __MATERIALINSTANCE_H__

#include "Goknar/Materials/Material.h"

class GOKNAR_API MaterialInstance : public Material
{
public:
	static void Create(const Material* parent);

	void PreInit();
	void Init();
	void PostInit();

protected:
	MaterialInstance();
	MaterialInstance(Material* parent);
	~MaterialInstance();

private:
};

#endif