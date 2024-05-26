#include "HUD.h"

#include "Math/GoknarMath.h"

HUD::HUD() : ObjectBase()
{
    SetIsTickable(false);
}

void HUD::PreInit()
{
    ObjectBase::PreInit();
}

void HUD::Init()
{
    ObjectBase::Init();
}

void HUD::PostInit()
{
    ObjectBase::PostInit();
}

void HUD::BeginGame()
{
    ObjectBase::BeginGame();
}

void HUD::Tick(float deltaTime)
{
	ObjectBase::Tick(deltaTime);

    refreshElapsedTime_ += deltaTime;
    if(refreshDeltaTime_ < refreshElapsedTime_)
    {
        UpdateHUD();
        refreshElapsedTime_ -= GoknarMath::Mod(refreshElapsedTime_, refreshDeltaTime_);
    }
}

void HUD::UpdateHUD()
{

}