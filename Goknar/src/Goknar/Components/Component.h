#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "Goknar/Core.h"

class ObjectBase;

class GOKNAR_API Component
{
public:
	Component() = delete;
	Component(const ObjectBase* parent);
	~Component()
	{

	}

	const ObjectBase* GetParent() const
	{
		return parent_;
	}

protected:

private:
	const ObjectBase* parent_;

};
#endif