#pragma once

#include "Goknar/Core.h"

class GOKNAR_API MaterialInitializer
{
public:
	MaterialInitializer() = delete;

	static void Init();

protected:
    
private:
	static void DefaultSceneAssets();
	static void DefaultCharacter();
};