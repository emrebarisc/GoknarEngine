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

	void PreInit() override;
	void Init() override;
	void PostInit() override;
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