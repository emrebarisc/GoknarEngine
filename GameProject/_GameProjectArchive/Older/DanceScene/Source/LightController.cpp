#include "LightController.h"

#include "Application.h"
#include "Engine.h"
#include "Scene.h"

#include "Goknar/Lights/SpotLight.h"

LightController::LightController()
{
	SetTickable(true);

	Scene* scene = engine->GetApplication()->GetMainScene();

	Vector3 spotLight1Position = Vector3(-20.f, -20.f, 20.f);
	spotLight1 = new SpotLight(15.f, 5.f);
	spotLight1->SetLightMobility(LightMobility::Movable);
	spotLight1->SetPosition(spotLight1Position);
	spotLight1->SetDirection(-spotLight1Position.GetNormalized());
	spotLight1->SetColor(Vector3(1.f, 0.2f, 0.2f));
	spotLight1->SetIntensity(500);
	scene->AddSpotLight(spotLight1);

	Vector3 spotLight2Position = Vector3(20.f, -20.f, 20.f);
	spotLight2 = new SpotLight(15.f, 5.f);
	spotLight2->SetLightMobility(LightMobility::Movable);
	spotLight2->SetPosition(spotLight2Position);
	spotLight2->SetDirection(-spotLight2Position.GetNormalized());
	spotLight2->SetColor(Vector3(0.2f, 1.f, 0.2f));
	spotLight2->SetIntensity(500);
	scene->AddSpotLight(spotLight2);

	Vector3 spotLight3Position = Vector3(-20.f, 20.f, 20.f);
	spotLight3 = new SpotLight(15.f, 5.f);
	spotLight3->SetLightMobility(LightMobility::Movable);
	spotLight3->SetPosition(spotLight3Position);
	spotLight3->SetDirection(-spotLight3Position.GetNormalized());
	spotLight3->SetColor(Vector3(1.f, 1.0f, 0.2f));
	spotLight3->SetIntensity(500);
	scene->AddSpotLight(spotLight3);

	Vector3 spotLight4Position = Vector3(20.f, 20.f, 20.f);
	spotLight4 = new SpotLight(15.f, 5.f);
	spotLight4->SetLightMobility(LightMobility::Movable);
	spotLight4->SetPosition(spotLight4Position);
	spotLight4->SetDirection(-spotLight4Position.GetNormalized());
	spotLight4->SetColor(Vector3(0.2f, 0.2f, 1.0f));
	spotLight4->SetIntensity(500);
	scene->AddSpotLight(spotLight4);

	Vector3 spotLight5Position = Vector3(20.f, -20.f, 20.f);
	spotLight5 = new SpotLight(20.f, 10.f);
	spotLight5->SetLightMobility(LightMobility::Static);
	spotLight5->SetPosition(spotLight5Position);
	spotLight5->SetDirection(-spotLight5Position.GetNormalized());
	spotLight5->SetColor(Vector3(1.f, 0.75f, 0.75f));
	spotLight5->SetIntensity(500);
	scene->AddSpotLight(spotLight5);

	angle = 0.f;
	radius = 10.f;
	rotationSpeed = 2.f;
}

void LightController::BeginPlay()
{
}

void LightController::Tick(float deltaTime)
{
	angle += rotationSpeed * deltaTime;
	Vector3 circle1 = Vector3(cos(angle), sin(angle), 0.f) * radius;
	Vector3 circle2 = Vector3(cos(5.f + angle), sin(5.f + angle), 0.f) * radius;
	Vector3 circle3 = Vector3(cos(angle), sin(10.f + angle), 0.f) * radius;
	Vector3 circle4 = Vector3(cos(15.f + angle), sin(angle), 0.f) * radius;

	spotLight1->SetDirection((-spotLight1->GetPosition() - Vector3(-1.f, -1.f, 0.f)) * sin(angle + 10.f) + circle1);
	spotLight2->SetDirection((-spotLight2->GetPosition() - Vector3(1.f, -1.f, 0.f)) * sin(angle) - circle2);
	spotLight3->SetDirection((-spotLight3->GetPosition() - Vector3(-1.f, 1.f, 0.f)) * sin(angle + 15.f) - circle3);
	spotLight4->SetDirection((-spotLight4->GetPosition() - Vector3(1.f, 1.f, 0.f)) * sin(angle + 5.f) + circle4);
}