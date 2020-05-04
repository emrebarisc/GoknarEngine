#pragma once

class Game;

class SpawnManager
{
public:
	friend Game;

	static SpawnManager* GetInstance();

private:
	SpawnManager();
	~SpawnManager();
	static SpawnManager* instance_;
};

