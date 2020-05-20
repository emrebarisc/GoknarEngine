#include "TimeDependentObject.h"

#include <functional>

typedef std::function<void()> TimerDelegate;

class GOKNAR_API Timer : public TimeDependentObject
{
public:
	Timer() :
		TimeDependentObject()
	{

	}

	~Timer()
	{

	}

	void CallOnTick(const TimerDelegate& function)
	{
		onOperate = function;
	}

protected:
	virtual void Operate()
	{
		onOperate();
	}

private:
	TimerDelegate onOperate;
};