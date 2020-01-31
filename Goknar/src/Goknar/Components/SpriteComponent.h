#ifndef __SPRITECOMPONENT_H__
#define __SPRITECOMPONENT_H__

#include "Component.h"

#include "Goknar/Core.h"

class SpriteMesh;
class MeshInstance;
class ObjectBase;

class GOKNAR_API SpriteComponent : public Component
{
public:
	SpriteComponent() = delete;
	SpriteComponent(ObjectBase* parent);
	~SpriteComponent();

	void SetSpriteMesh(SpriteMesh* sprite);

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