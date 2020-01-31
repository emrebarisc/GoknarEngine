#ifndef __ANIMATEDSPRITEMESH_H__
#define __ANIMATEDSPRITEMESH_H__

#include "SpriteMesh.h"
#include "Goknar/TimeDependentObject.h"

class Component;

class GOKNAR_API AnimatedSpriteMesh : public SpriteMesh, public TimeDependentObject
{
public:
	AnimatedSpriteMesh();

	~AnimatedSpriteMesh()
	{

	}

	void Init() override;
private:
	void Operate() override;
};

#endif