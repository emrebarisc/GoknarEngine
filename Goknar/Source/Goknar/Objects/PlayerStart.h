#ifndef __PLAYERSTART_H__
#define __PLAYERSTART_H__

#include "Goknar/ObjectBase.h"

class GOKNAR_API PlayerStart : public ObjectBase
{
public:
	PlayerStart(const ObjectInitializer& objectInitializer = ObjectInitializer());
	~PlayerStart() override;

	ObjectBase* Clone() const override;

	void PreInit() override;
	void Init() override;
	void PostInit() override;

protected:

private:
};

#endif
