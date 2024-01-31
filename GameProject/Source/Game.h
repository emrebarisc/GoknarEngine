#include "Goknar/Application.h"

class Archer;
class PhysicsArcher;
class Dancer;
class LightController;
class Sun;
class Fire;
class RandomGrassSpawner;
class MaterialSphereSpawner;
class PhysicsObjectSpawner;
class Terrain;

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

	PhysicsArcher* GetPhysicsArcher() const
	{
		return physicsArcher_;
	}

private:
	Archer* archer_;
	PhysicsArcher* physicsArcher_;
	Dancer* dancer_;
	LightController* lightController_;
	Sun* sun_;
	Fire* fire_;
	RandomGrassSpawner* randomGrassSpawner_;
	MaterialSphereSpawner* materialSphereSpawner_;

	PhysicsObjectSpawner* physicsObjectSpawner_;

	Terrain* terrain;
};