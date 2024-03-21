#include "TimeDependentObject.h"

#include "Delegates/Delegate.h"

class GOKNAR_API Timer : public TimeDependentObject
{
public:
	Timer();
	~Timer() {}

	virtual void Tick(float deltaSecond) override;

	void CallOnTick(const Delegate<void()>& function)
	{
		onOperate_ = function;
	}

	float GetTicksPerSecond() const
	{
		return ticksPerSecond_;
	}

	void SetTicksPerSecond(float ticksPerSecond)
	{
		ticksPerSecond_ = ticksPerSecond;
		timeToRefreshTimeVariables_ = 1.f / ticksPerSecond;
	}

	void SetTimeToTick(float timeInSeconds)
	{
		ticksPerSecond_ = 1.f / timeInSeconds;
		timeToRefreshTimeVariables_ = timeInSeconds;
	}

	virtual void SetIsActive(bool isActive) override;

	virtual void Reset()
	{
		elapsedTime_ = 0.f;
	}

protected:
	virtual void Operate()
	{
		if (!onOperate_.isNull())
		{
			onOperate_();
		}
	}

private:
	Delegate<void()> onOperate_;

	float ticksPerSecond_{ 30.f };
	float timeToRefreshTimeVariables_{ 1.f / ticksPerSecond_ };
	float elapsedTime_{ 0.f };
};