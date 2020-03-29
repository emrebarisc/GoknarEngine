#ifndef __ANIMATEDSPRITECOMPONENT_H__
#define __ANIMATEDSPRITECOMPONENT_H__

#include "Component.h"

#include "Goknar/Core.h"

class AnimatedSpriteMesh;
class MeshInstance;
class ObjectBase;

class GOKNAR_API AnimatedSpriteComponent : public Component
{
public:
	AnimatedSpriteComponent() = delete;
	AnimatedSpriteComponent(ObjectBase* parent);
	~AnimatedSpriteComponent();

	void SetMesh(AnimatedSpriteMesh* sprite);

	const Matrix& GetRelativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	const Matrix& GetRerlativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	void WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix) override;

protected:
	inline void UpdateRelativeTransformationMatrix() override;

private:
	MeshInstance* meshInstance_;
};
#endif