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

	int GetTicksPerSecond() const
	{
		return ticksPerSecond_;
	}

	void SetTicksPerSecond(int ticksPerSecond)
	{
		ticksPerSecond_ = ticksPerSecond;
		timeToRefreshTimeVariables_ = 1.f / ticksPerSecond;
	}

	virtual void SetIsActive(bool isActive) override;

protected:
	virtual void Operate()
	{
		onOperate_();
	}

private:
	Delegate<void()> onOperate_;

	int ticksPerSecond_{ 30 };
	float timeToRefreshTimeVariables_{ 1.f / ticksPerSecond_ };
	float elapsedTime_{ 0.f };
};