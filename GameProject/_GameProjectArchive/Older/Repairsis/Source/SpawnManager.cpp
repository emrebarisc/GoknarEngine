#include "SpawnManager.h"

SpawnManager* SpawnManager::instance_ = nullptr;

SpawnManager* SpawnManager::GetInstance() 
{
	if (!instance_)
		instance_ = new SpawnManager();

	return instance_;
}

SpawnManager::SpawnManager()
{
}


SpawnManager::~SpawnManager()
{
}
