#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "Goknar/Core.h"
#include "Goknar/Math/Matrix.h"

class ObjectBase;

class GOKNAR_API Component
{
public:
	Component() = delete;
	Component(ObjectBase* parent);
	virtual ~Component()
	{

	}

	virtual void Destroy() = 0;

	const ObjectBase* GetParent() const
	{
		return parent_;
	}

protected:
private:
	ObjectBase* parent_;
};
#endif