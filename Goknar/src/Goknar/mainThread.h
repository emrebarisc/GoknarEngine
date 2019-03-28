#pragma once

int main(int argc, char **argv)
{
	Engine *mainEngine = new Engine();
	mainEngine->SetApplication(Goknar::CreateApplication());
	mainEngine->Init();
	mainEngine->BeginGame();
	mainEngine->Run();
	delete mainEngine;
	
	return 0;
}