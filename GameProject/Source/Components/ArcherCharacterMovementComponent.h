#ifndef __ARCHERCHARACTERMOVEMENTCOMPONENT_H__
#define __ARCHERCHARACTERMOVEMENTCOMPONENT_H__

#include "Goknar/Physics/Components/CharacterMovementComponent.h"
#include "Goknar/Math/InterpolatingValue.h"

class ArcherCharacter;
class Camera;

class ArcherCharacterMovementComponent : public CharacterMovementComponent
{
public:
	ArcherCharacterMovementComponent(Component* parent);
	~ArcherCharacterMovementComponent()
	{

	}

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;

	virtual void AddMovementDirection(const Vector3& movementDirection);

private:
	void OnMovementDirectionInterpolated();
	void OnMovementRotationInterpolated();

	InterpolatingValue<Vector3> movementDirection_;
	InterpolatingValue<float> movementRotation_;

	ArcherCharacter* ownerArcher_{ nullptr };
	Camera* thirdPersonCamera_{ nullptr };
};

#endif