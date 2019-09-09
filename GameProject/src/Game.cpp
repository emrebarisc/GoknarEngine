#include <Goknar.h>

#include "Goknar/Lights/PointLight.h"

class Game : public Application
{
public:
	Game();

	~Game() = default;

	void Run() override;

private:
	PointLight* newPointLight1;
	PointLight* newPointLight2;

	DirectionalLight* newDirectionalLight;

	SpotLight* newSpotLight1;
	SpotLight* newSpotLight2;
};

Game::Game() : Application()
{
	//mainScene_->ReadSceneData("../GameProject/Content/Scenes/SphereScene.xml");
	//mainScene_->ReadSceneData("../GameProject/Content/Scenes/MonkeyScene.xml");
	//mainScene_->ReadSceneData("../GameProject/Content/Scenes/ThreeSpheres.xml");
	mainScene_->ReadSceneData("../GameProject/Content/Scenes/ThreeDifferentShapes.xml");
	//mainScene_->ReadSceneData("../GameProject/Content/Scenes/SphereWithNormals.xml");

	newPointLight1 = new PointLight();
	newPointLight1->SetPosition(Vector3(-7.5f, 7.5f, 7.5f));
	newPointLight1->SetColor(Vector3(1.f, 1.f, 1.f));
	newPointLight1->SetIntensity(50.f);
	newPointLight1->SetLightMobility(LightMobility::Movable);
	mainScene_->AddPointLight(newPointLight1);

	/*newPointLight2 = new PointLight();
	newPointLight2->SetPosition(Vector3(7.5f, -7.5f, 7.5f));
	newPointLight2->SetColor(Vector3(1.f, 1.f, 1.f));
	newPointLight2->SetIntensity(50.f);
	newPointLight2->SetLightMobility(LightMobility::Movable);
	mainScene_->AddPointLight(newPointLight2);

	newDirectionalLight = new DirectionalLight();
	newDirectionalLight->SetDirection(Vector3(1.f, 0.8f, -1.f));
	newDirectionalLight->SetColor(Vector3(1.f, 1.f, 1.f));
	newDirectionalLight->SetIntensity(0.5f);
	newDirectionalLight->SetLightMobility(LightMobility::Movable);
	mainScene_->AddDirectionalLight(newDirectionalLight);*/
	
	newSpotLight1 = new SpotLight();
	newSpotLight1->SetPosition(Vector3(15.f, 15.f, 15.f));
	newSpotLight1->SetDirection(Vector3(-1.f, -1.f, -1.f));
	newSpotLight1->SetColor(Vector3(1.f, 0.f, 0.f));
	newSpotLight1->SetIntensity(200.f);
	newSpotLight1->SetCoverageAngle(10);
	newSpotLight1->SetFalloffAngle(8);
	newSpotLight1->SetLightMobility(LightMobility::Movable);
	mainScene_->AddSpotLight(newSpotLight1);

	newSpotLight2 = new SpotLight();
	newSpotLight2->SetPosition(Vector3(-15.f, -15.f, 15.f));
	newSpotLight2->SetDirection(Vector3(1.f, 1.f, -1.f));
	newSpotLight2->SetColor(Vector3(1.f, 1.f, 0.6f));
	newSpotLight2->SetIntensity(200.f);
	newSpotLight2->SetCoverageAngle(8);
	newSpotLight2->SetFalloffAngle(4);
	newSpotLight2->SetLightMobility(LightMobility::Movable);
	mainScene_->AddSpotLight(newSpotLight2);
}

void Game::Run()
{
	static float delay = 0.f;

	static Vector3 initialDirection1 = newSpotLight1->GetDirection();
	static Vector3 othonormalBasis1 = newSpotLight1->GetDirection().GetOrthonormalBasis();

	static Vector3 initialDirection2 = newSpotLight2->GetDirection();
	static Vector3 othonormalBasis2 = newSpotLight2->GetDirection().GetOrthonormalBasis();

	Vector3 newDirection1 = initialDirection1 + othonormalBasis1 * sin(delay) * 0.25f;
	newSpotLight1->SetDirection(newDirection1);

	Vector3 newDirection2 = initialDirection2 + othonormalBasis2 * sin(delay) * 0.25f;
	newSpotLight2->SetDirection(newDirection2);


	delay += 0.0025f;

	static float theta = 0.f;
	static float radius = 10.f;
	theta += 0.0025f;
	Vector3 pointLightPosition = Vector3(radius * cos(theta), radius * sin(theta), 10.f);
	newPointLight1->SetPosition(pointLightPosition);
}

Application *CreateApplication()
{
	return new Game();
}
