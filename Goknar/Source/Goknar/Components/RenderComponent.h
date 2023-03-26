#ifndef __RENDERCOMPONENT_H__
#define __RENDERCOMPONENT_H__

#include "Goknar/Core.h"
#include "Goknar/Components/Component.h"
#include "Goknar/Math/Matrix.h"

class ObjectBase;

class GOKNAR_API RenderComponent : public Component
{
public:
	RenderComponent() = delete;
	RenderComponent(Component* parent);
	virtual ~RenderComponent()
	{

	}

	virtual void Destroy() = 0;
	virtual void SetIsActive(bool isRendered) = 0;
	virtual void WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix) override = 0;

protected:
private:
};
#endif