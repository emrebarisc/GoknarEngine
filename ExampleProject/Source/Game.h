#include "Goknar/Application.h"

class DefaultCharacter;
class FreeCameraObject;
class GameState;

class GOKNAR_API Game : public Application
{
public:
	Game();

	~Game()
	{
	}

	void Run() override;

	DefaultCharacter* GetDefaultCharacter() const
	{
		return defaultCharacter_;
	}

	FreeCameraObject* GetFreeCameraObject() const
	{
		return freeCameraObject_;
	}

	GameState* GetGameState() const
	{
		return gameState_;
	}

private:
	DefaultCharacter* defaultCharacter_{ nullptr };
	FreeCameraObject* freeCameraObject_{ nullptr };
	GameState* gameState_{ nullptr };
};