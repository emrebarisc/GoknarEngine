#pragma once

#include "Goknar/Factories/DynamicObjectFactory.h"

#include "Objects/FloatingObject.h"
#include "Objects/Terrain.h"
#include "Objects/TerrainGrassSpawner.h"


class GOKNAR_API FactoryRegistrar
{
public:
	static void Register()
	{
		REGISTER_CLASS(Terrain);
		REGISTER_CLASS(TerrainGrassSpawner);
		REGISTER_CLASS(FloatingObject);
	}
};