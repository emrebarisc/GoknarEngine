#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "Core.h"

class GOKNAR_API Controller
{
public:
	Controller();
	virtual ~Controller();

	virtual void BeginGame() {}
	virtual void SetupInputs() = 0;

	virtual void SetIsActive(bool isActive)
	{
		isActive_ = isActive;
	}

	bool GetIsActive() const
	{
		return isActive_;
	}
private:
	bool isActive_{ true };
};

#endif