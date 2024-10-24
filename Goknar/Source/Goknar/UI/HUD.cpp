#include "HUD.h"

#include "Engine.h"
#include "Math/GoknarMath.h"

HUD::HUD() : ObjectBase()
{
    SetIsTickable(false);
    SetName("HUD");
}

HUD::~HUD() 
{
    if(engine->GetHUD() == this)
    {
        engine->SetHUD(nullptr);
    }
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
}

void HUD::SetIsTickable(bool isTickable)
{
    if (!GetIsInitialized())
    {
        ObjectBase::SetIsTickable(false);

        return;
    }

    GOKNAR_ASSERT(false, "HUD cannot be set as tick enabled.");
}

void HUD::HUDTick(float deltaTime)
{
    refreshElapsedTime_ += deltaTime;
    if (refreshDeltaTime_ < refreshElapsedTime_)
    {
        UpdateHUD();
        refreshElapsedTime_ -= GoknarMath::Mod(refreshElapsedTime_, refreshDeltaTime_);
    }
}

void HUD::UpdateHUD()
{

}