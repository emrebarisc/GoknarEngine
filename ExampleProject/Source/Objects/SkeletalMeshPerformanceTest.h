#pragma once

#include "Goknar/ObjectBase.h" 

class GOKNAR_API SkeletalMeshPerformanceTest : public ObjectBase
{
public:
	SkeletalMeshPerformanceTest();

	virtual void BeginGame() override;

	size_t objectCount{ 0 };
	int marginBetween{ 2 };

protected:

private:

};