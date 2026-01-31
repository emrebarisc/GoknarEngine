#pragma once

#include "Goknar/Core.h"
#include "Goknar/Delegates/MulticastDelegate.h"

class Game;

class GOKNAR_API GameState
{
public:
	GameState();

	void StartGame();
	void ResumeGame();
	void PauseGame();
	void RestartGame();
	void GameOver();

	MulticastDelegate<void()> OnGameStartedDelegate;
	MulticastDelegate<void()> OnGamePausedDelegate;
	MulticastDelegate<void()> OnGameResumedDelegate;
	MulticastDelegate<void()> OnGameRestartedDelegate;
	MulticastDelegate<void()> OnGameOverDelegate;

	bool GetIsGamePaused() const
	{
		return isGamePaused_;
	}

protected:
    
private:
	bool isGamePaused_{ false };

	Game* game_;
};