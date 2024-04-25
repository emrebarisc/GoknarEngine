#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "Core.h"
#include "Contents/Content.h"

class GOKNAR_API Audio : public Content
{
public:
	Audio();
	virtual ~Audio() = default;

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

private:
};

#endif