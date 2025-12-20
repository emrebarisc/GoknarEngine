#ifndef __TICKER_H__
#define __TICKER_H__

#include "TimeDependentObject.h"

#include "Delegates/Delegate.h"

#include "Goknar/Log.h"

class GOKNAR_API Ticker : public TimeDependentObject
{
public:
	Ticker();
	virtual ~Ticker()
	{
		GOKNAR_INFO("~Ticker");
	}

	virtual void Tick(float deltaSecond) override;

	void CallOnTick(const Delegate<void(float)>& function)
	{
		onOperate_ = function;
	}

protected:
	virtual void Operate(float deltaTime)
	{
		if (!onOperate_.isNull())
		{
			onOperate_(deltaTime);
		}
	}

private:
	Delegate<void(float)> onOperate_;
};

#endif