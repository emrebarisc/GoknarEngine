#ifndef __GAMEMANAGER_H__
#define __GAMEMANAGER_H__

class GameManager
{
public:
	static GameManager* GetInstance()
	{
		if (instance_ == nullptr)
		{
			instance_ = new GameManager();
		}

		return instance_;
	}

	void Restart();
	void GameOver();
	void IncreaseScore(float amount = 1.f);

	bool GetIsGameOver() const
	{
		return isGameOver_;
	}

private:
	GameManager() : isGameOver_(false), score_(0.f)
	{

	}

	~GameManager()
	{

	}

	static GameManager* instance_;

	float score_;

	bool isGameOver_;
};

#endif