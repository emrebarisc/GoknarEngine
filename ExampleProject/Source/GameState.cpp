#include "GameState.h"

#include "Goknar/Engine.h"

#include "Game.h"

GameState::GameState()
{
    game_ = dynamic_cast<Game*>(engine->GetApplication());
}

void GameState::StartGame()
{
    engine->SetTimeScale(1.f);

    isGamePaused_ = false;

    OnGameStartedDelegate();
}

void GameState::PauseGame()
{
    engine->SetTimeScale(0.f);

    isGamePaused_ = true;

    OnGamePausedDelegate();
}

void GameState::ResumeGame()
{
    engine->SetTimeScale(1.f);

    isGamePaused_ = false;

    OnGameResumedDelegate();
}

void GameState::RestartGame()
{
    engine->SetTimeScale(1.f);

    isGamePaused_ = false;

    OnGameRestartedDelegate();
}

void GameState::GameOver()
{
    engine->SetTimeScale(0.f);

    isGamePaused_ = true;

    OnGameOverDelegate();
}