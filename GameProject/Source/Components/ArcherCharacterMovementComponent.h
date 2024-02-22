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

	virtual void AddMovementDirection(const Vector3& movementDirection)
	{
		Vector3 newMovementDirection = movementDirection_.current + movementDirection;

		if (EPSILON < newMovementDirection.Length())
		{
			const float newDestinationAngle = atan2f(newMovementDirection.y, newMovementDirection.x);

			float destinationAngle = newDestinationAngle - movementRotation_.current;
			if (PI < destinationAngle)
			{
				destinationAngle -= TWO_PI;
			}
			else if (destinationAngle <= -PI)
			{
				destinationAngle += TWO_PI;
			}

			movementRotation_ = destinationAngle;
		}

		movementDirection_ = newMovementDirection;
	}

private:
	void OnMovementDirectionInterpolated();

	InterpolatingValue<Vector3> movementDirection_;
	InterpolatingValue<float> movementRotation_;

	ArcherCharacter* ownerArcher_{ nullptr };
	Camera* thirdPersonCamera_{ nullptr };
};

#endif