#include "Sun.h"

#include "Application.h"
#include "Engine.h"
#include "Scene.h"
#include "Lights/DirectionalLight.h"
#include "Math/GoknarMath.h"

Sun::Sun() :
	ObjectBase()
{
	SetIsTickable(false);

	sunLight_ = new DirectionalLight();
	sunLight_->SetColor(Vector3(1.0f, 0.99f, 0.83f));
	sunLight_->SetIntensity(1.f);
	sunLight_->SetIsShadowEnabled(true);
	sunLight_->SetLightMobility(LightMobility::Dynamic);
	sunLight_->SetShadowWidth(4096);
	sunLight_->SetShadowHeight(4096);
	sunLight_->SetShadowBiasValue(0.0005f);
	sunLight_->SetShadowIntensity(0.5f);
}

Sun::~Sun()
{
	delete sunLight_;
}

void Sun::BeginGame()
{
	sunLight_->SetDirection(sunLightDirection_);
	sunRotationAxis_ = Vector3::Cross(sunLightDirection_, Vector3::Cross(sunLightDirection_.GetOrthonormalBasis(), sunLightDirection_));
}

void Sun::Tick(float deltaTime)
{
	sunLightDirection_ = sunLightDirection_.RotateVectorAroundAxis(sunRotationAxis_, sunLightRotationSpeed_ * deltaTime);
	sunLight_->SetDirection(sunLightDirection_);
}
