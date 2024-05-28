#pragma once

#include "Goknar/Core.h"
#include "Goknar/ObjectBase.h"

class GOKNAR_API HUD : public ObjectBase
{
public:
	HUD();
    virtual ~HUD();

    virtual void PreInit() override;
    virtual void Init() override;
    virtual void PostInit() override;

    virtual void BeginGame() override;
    virtual void Tick(float deltaTime) override final;

    virtual void UpdateHUD();

    void SetRefreshFPS(int refreshFPS)
    {
        refreshFPS_ = refreshFPS;
        refreshDeltaTime_ = 1.f / refreshFPS;
    }

    int GetRefresFPS() const
    {
        return refreshFPS_;
    }

    float GetRefresDeltaTime() const
    {
        return refreshDeltaTime_;
    }

protected:

private:
    int refreshFPS_{ 30 };
    float refreshDeltaTime_{ 1.f / refreshFPS_ };
    float refreshElapsedTime_{ 0.f };
};