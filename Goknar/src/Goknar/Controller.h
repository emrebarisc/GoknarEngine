#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "Core.h"

class GOKNAR_API Controller
{
public:
	Controller();
	~Controller();

	virtual void SetupInputs() = 0;
private:
};

#endif