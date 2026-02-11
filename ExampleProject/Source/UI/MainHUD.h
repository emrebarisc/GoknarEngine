#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "Goknar/Core.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Delegates/Delegate.h"
#include "Goknar/UI/HUD.h"

#include "UI/Panels/Panel.h"

class Image;

class UIContext;

class GOKNAR_API MainHUD : public HUD
{
public:
	MainHUD();
	MainHUD(const MainHUD&) = delete;
	MainHUD(MainHUD&&) = default;
	virtual ~MainHUD();

	MainHUD& operator=(const MainHUD&) = delete;
	MainHUD& operator=(MainHUD&&) = default;

    virtual void PreInit() override;
    virtual void Init() override;
    virtual void PostInit() override;

    virtual void BeginGame() override;

    virtual void UpdateHUD() override;

	template<typename T>
	void ShowPanel();

	template<typename T>
	void HidePanel();

	const std::vector<std::unique_ptr<IPanel>>& GetPanels() const
	{
		return panels_;
	}

	template<typename T>
	IPanel* GetPanel() const
	{
		std::string mapKey = std::string(typeid(T).name());
		auto it = panelIndexMap_.find(mapKey);

		if (it == panelIndexMap_.end())
		{
			return nullptr;
		}

		return panels_[panelIndexMap_.at(mapKey)].get();
	}

protected:

private:
	void OnKeyboardEvent(int key, int scanCode, int action, int mod);

	void OnCursorMove(double xPosition, double yPosition);
	void OnScroll(double xOffset, double yOffset);
	void OnLeftClickPressed();
	void OnLeftClickReleased();
	void OnCharPressed(unsigned int codePoint);
	void OnWindowSizeChanged(int width, int height);

	void OnGamePaused();
	void OnGameResumed();

	void DrawMainHUD();

	Delegate<void(int, int, int, int)> onKeyboardEventDelegate_;
	Delegate<void(double, double)> onCursorMoveDelegate_;
	Delegate<void(double, double)> onScrollDelegate_;
	Delegate<void()> onLeftClickPressedDelegate_;
	Delegate<void()> onLeftClickReleasedDelegate_;
	Delegate<void(unsigned int)> onCharPressedDelegate_;

	Delegate<void(int, int)> onWindowSizeChangedDelegate_;

	Delegate<void()> onDeleteInputPressedDelegate_;
	Delegate<void()> onFocusInputPressedDelegate_;
	Delegate<void()> onCancelInputPressedDelegate_;

	Image* uiImage_;

	template<typename T>
	void AddPanel();

	std::vector<std::unique_ptr<IPanel>> panels_;
	std::unordered_map<std::string, int> panelIndexMap_;

	UIContext* context_;
};

template<typename T>
inline void MainHUD::AddPanel()
{
	panels_.emplace_back(std::make_unique<T>(this));
	panelIndexMap_[typeid(T).name()] = panels_.size() - 1;
}

template<typename T>
inline void MainHUD::ShowPanel()
{
	std::string mapKey = typeid(T).name();
	GOKNAR_ASSERT(panelIndexMap_.find(mapKey) != panelIndexMap_.end());
	
	panels_[panelIndexMap_[typeid(T).name()]]->SetIsOpen(true);
}

template<typename T>
inline void MainHUD::HidePanel()
{
	std::string mapKey = typeid(T).name();
	GOKNAR_ASSERT(panelIndexMap_.find(mapKey) != panelIndexMap_.end());

	panels_[panelIndexMap_[typeid(T).name()]]->SetIsOpen(false);
}
