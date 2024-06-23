#include "Fire.h"

#include "Application.h"
#include "Engine.h"
#include "Scene.h"
#include "Lights/PointLight.h"
#include "Math/GoknarMath.h"
#include "Physics/PhysicsWorld.h"

Fire::Fire() :
	ObjectBase()
{
	SetIsTickable(true);

	fireLight_ = new PointLight();
	fireLight_->SetColor(Vector3{ 0.7605246901512146f, 0.09758714586496353f, 0.015996338799595833f });
	fireLight_->SetIntensity(fireIntensity_);
	fireLight_->SetRadius(25.f);
	fireLight_->SetIsShadowEnabled(true);
	fireLight_->SetLightMobility(LightMobility::Dynamic);
	engine->GetApplication()->GetMainScene()->AddPointLight(fireLight_);
}

void Fire::BeginGame()
{
	RaycastData raycastData;
	raycastData.from = Vector3{ 10.f, 0.f, 1000.f };
	raycastData.to = Vector3{ 10.f, 0.f, -1000.f };

	RaycastSingleResult raycastResult;

	if (engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult))
	{
		SetWorldPosition(raycastResult.hitPosition + Vector3{ 0.f, 0.f, 3.f });
	}
}

void Fire::Tick(float deltaTime)
{
	float scaledTime = engine->GetElapsedTime() * 2.f;
	float sinValue = sin(scaledTime);
	float cosValue = cos(scaledTime);

	float multiplier = (0.9f + tan(cos(scaledTime)) * tan(cos(GoknarMath::Pow(scaledTime, 1.25f))) * 0.1f);

	fireLight_->SetIntensity(fireIntensity_ * multiplier);
}

void Fire::SetWorldPosition(const Vector3& position, bool updateWorldTransformationMatrix)
{
	ObjectBase::SetWorldPosition(position, updateWorldTransformationMatrix);

	fireLight_->SetPosition(position);
}
