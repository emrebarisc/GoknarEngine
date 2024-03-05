#include "Goknar/Application.h"

class Archer;
class ArcherCharacter;
class Dancer;
class LightController;
class Sun;
class Fire;
class RandomGrassSpawner;
class MaterialSphereSpawner;
class PhysicsObjectSpawner;
class Terrain;
class FreeCameraObject;
class PhysicsBox;

class GOKNAR_API Game : public Application
{
public:
	Game();

	~Game()
	{
	}

	void Run() override;

	Archer* GetArcher() const
	{
		return archer_;
	}

	ArcherCharacter* GetPhysicsArcher() const
	{
		return physicsArcher_;
	}

	FreeCameraObject* GetFreeCameraObject() const
	{
		return freeCameraObject_;
	}

private:
	Archer* archer_{ nullptr };
	ArcherCharacter* physicsArcher_{ nullptr };
	Dancer* dancer_{ nullptr };
	LightController* lightController_{ nullptr };
	Sun* sun_{ nullptr };
	Fire* fire_{ nullptr };
	RandomGrassSpawner* randomGrassSpawner_{ nullptr };
	MaterialSphereSpawner* materialSphereSpawner_{ nullptr };
	FreeCameraObject* freeCameraObject_;

	PhysicsObjectSpawner* physicsObjectSpawner_{ nullptr };
	PhysicsBox* boxFloor_{ nullptr };

	Terrain* terrain{ nullptr };
};