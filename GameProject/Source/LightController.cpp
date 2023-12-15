#include "LightController.h"

#include "Application.h"
#include "Engine.h"
#include "Scene.h"

#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"

LightController::LightController()
{
	SetIsTickable(false);

	Scene* scene = engine->GetApplication()->GetMainScene();

/*	Vector3 spotLight1Position = centerPosition_ + Vector3(-10.f, -10.f, 10.f);
	spotLight1_ = new SpotLight(15.f, 5.f);
	spotLight1_->SetLightMobility(LightMobility::Dynamic);
	spotLight1_->SetPosition(spotLight1Position);
	spotLight1_->SetDirection((centerPosition_ - spotLight1Position).GetNormalized());
	spotLight1_->SetColor(Vector3(1.f, 0.2f, 0.2f));
	spotLight1_->SetIntensity(250.f);
	spotLight1_->SetIsShadowEnabled(true);
	scene->AddSpotLight(spotLight1_);

	Vector3 spotLight2Position = centerPosition_ + Vector3(10.f, -10.f, 10.f);
	spotLight2_ = new SpotLight(15.f, 5.f);
	spotLight2_->SetLightMobility(LightMobility::Dynamic);
	spotLight2_->SetPosition(spotLight2Position);
	spotLight2_->SetDirection((centerPosition_ - spotLight2Position).GetNormalized());
	spotLight2_->SetColor(Vector3(0.2f, 1.f, 0.2f));
	spotLight2_->SetIntensity(250.f);
	spotLight2_->SetIsShadowEnabled(true);
	scene->AddSpotLight(spotLight2_);

	Vector3 spotLight3Position = centerPosition_ + Vector3(-10.f, 10.f, 10.f);
	spotLight3_ = new SpotLight(15.f, 5.f);
	spotLight3_->SetLightMobility(LightMobility::Dynamic);
	spotLight3_->SetPosition(spotLight3Position);
	spotLight3_->SetDirection((centerPosition_ - spotLight3Position).GetNormalized());
	spotLight3_->SetColor(Vector3(1.f, 1.0f, 0.2f));
	spotLight3_->SetIntensity(250.f);
	spotLight3_->SetIsShadowEnabled(true);
	scene->AddSpotLight(spotLight3_);

	Vector3 spotLight4Position = centerPosition_ + Vector3(10.f, 10.f, 10.f);
	spotLight4_ = new SpotLight(15.f, 5.f);
	spotLight4_->SetLightMobility(LightMobility::Dynamic);
	spotLight4_->SetPosition(spotLight4Position);
	spotLight4_->SetDirection((centerPosition_ - spotLight4Position).GetNormalized());
	spotLight4_->SetColor(Vector3(0.2f, 0.2f, 1.0f));
	spotLight4_->SetIntensity(250.f);
	spotLight4_->SetIsShadowEnabled(true);
	scene->AddSpotLight(spotLight4_);

	Vector3 spotLight5Position = centerPosition_ + Vector3(10.f, -10.f, 10.f);
	spotLight5_ = new SpotLight(20.f, 5.f);
	spotLight5_->SetLightMobility(LightMobility::Static);
	spotLight5_->SetPosition(spotLight5Position);
	spotLight5_->SetDirection((centerPosition_ - spotLight5Position).GetNormalized());
	spotLight5_->SetColor(Vector3(1.f, 0.75f, 0.75f));
	spotLight5_->SetIntensity(100.f);
	spotLight5_->SetIsShadowEnabled(true);
	scene->AddSpotLight(spotLight5_);
*/

	float xInitial = -20.f;
	float yInitial = -20.f;
	float xDiff = 5.f;
	float yDiff = 5.f;
	float z = 3.f;

	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			Vector3 position{ xInitial - x * xDiff, yInitial - y * yDiff, z};

			PointLight* pointLight = new PointLight();
			pointLight->SetPosition(position);
			pointLight->SetIntensity(10.f);
			pointLight->SetIsShadowEnabled(false);
			pointLight->SetLightMobility(LightMobility::Static);
			scene->AddPointLight(pointLight);

			ObjectBase* boxObject = new ObjectBase();
			boxObject->SetWorldPosition(position - Vector3{0.f, 0.f, z});
			StaticMeshComponent* boxComponent = boxObject->AddSubComponent<StaticMeshComponent>();
			StaticMesh* staticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Cube.fbx");
			boxComponent->SetMesh(staticMesh);
		}
	}

	angle_ = 0.f;
	radius_ = 10.f;
	rotationSpeed_ = 2.f;
}

void LightController::BeginGame()
{
}

void LightController::Tick(float deltaTime)
{
	angle_ += rotationSpeed_ * deltaTime;
	Vector3 circle1 = centerPosition_ + Vector3(cos(angle_), 0.f, 0.f) * radius_;
	Vector3 circle2 = centerPosition_ + Vector3(cos(5.f + angle_), 0.f, 0.f) * radius_;
	Vector3 circle3 = centerPosition_ + Vector3(0.f, sin(10.f + angle_), 0.f) * radius_;
	Vector3 circle4 = centerPosition_ + Vector3(cos(15.f + angle_), 0.f, 0.f) * radius_;

	spotLight1_->SetDirection((Vector3(-1.f, -1.f, 0.f) * sin(angle_ + 10.f) + circle1) - spotLight1_->GetPosition());
	spotLight2_->SetDirection((Vector3(1.f, -1.f, 0.f) * sin(angle_) + circle2) - spotLight2_->GetPosition());
	spotLight3_->SetDirection((Vector3(-1.f, 1.f, 0.f) * sin(angle_ + 15.f) + circle3) - spotLight3_->GetPosition());
	spotLight4_->SetDirection((Vector3(1.f, 1.f, 0.f) * sin(angle_ + 5.f) + circle4) - spotLight4_->GetPosition());
}