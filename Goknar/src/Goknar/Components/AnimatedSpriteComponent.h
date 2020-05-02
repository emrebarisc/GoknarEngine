#ifndef __ANIMATEDSPRITECOMPONENT_H__
#define __ANIMATEDSPRITECOMPONENT_H__

#include "MeshComponent.h"

#include "Goknar/Core.h"

class AnimatedSpriteMesh;
class MeshInstance;
class ObjectBase;

class GOKNAR_API AnimatedSpriteComponent : public MeshComponent
{
public:
	AnimatedSpriteComponent() = delete;
	AnimatedSpriteComponent(ObjectBase* parent);
	virtual ~AnimatedSpriteComponent();

	void SetMesh(Mesh* sprite) override;

	const Matrix& GetRelativeTransformationMatrix() const
	{
		return relativeTransformationMatrix_;
	}

	void WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix) override;

	AnimatedSpriteMesh* GetAnimatedSpriteMesh()
	{
		return animatedSpriteMesh_;
	}

protected:
	inline void UpdateRelativeTransformationMatrix() override;

private:

	AnimatedSpriteMesh* animatedSpriteMesh_;
};
#endif