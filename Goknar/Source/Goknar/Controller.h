#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "ObjectBase.h"

class GOKNAR_API Controller : public ObjectBase
{
public:
	Controller();
	virtual ~Controller();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	virtual void BeginGame() {}
	virtual void SetupInputDelegates() = 0;
private:

};

#endif