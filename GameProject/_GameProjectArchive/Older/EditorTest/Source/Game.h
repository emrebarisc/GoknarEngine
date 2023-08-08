#pragma once

#include <Goknar/Application.h>

class CameraController;

class Game : public Application
{
public:
	Game();

	~Game();

	void Run() override;

private:
	CameraController* cameraController_;
};