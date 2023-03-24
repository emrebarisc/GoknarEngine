#ifndef __ARCHERMOVEMENTCOMPONENT_H__
#define __ARCHERMOVEMENTCOMPONENT_H__

#include "Goknar/Components/RenderComponent.h"

class ArcherMovementComponent/* : public RenderComponent*/
{
public:
	ArcherMovementComponent(ObjectBase* parent);

	void AddMovementVector(const Vector3& movement)
	{
		movementVector += movement;
		movementVector.Normalize();
	}

protected:

private:
	Vector3 movementVector;
};


#endif