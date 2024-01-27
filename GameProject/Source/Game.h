#include "Goknar/Application.h"

class Archer;
class Dancer;
class LightController;
class Sun;
class Fire;
class RandomGrassSpawner;
class MaterialSphereSpawner;
class PhysicsObjectSpawner;

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

private:
	Archer* archer_;
	Dancer* dancer_;
	LightController* lightController_;
	Sun* sun_;
	Fire* fire_;
	RandomGrassSpawner* randomGrassSpawner_;
	MaterialSphereSpawner* materialSphereSpawner_;

	PhysicsObjectSpawner* physicsObjectSpawner_;
};