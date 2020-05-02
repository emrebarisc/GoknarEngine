#pragma once
class SpawnManager
{
public:
	SpawnManager* GetInstance();

private:
	SpawnManager();
	~SpawnManager();
	static SpawnManager* instance_;
};

