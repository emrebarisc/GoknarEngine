#pragma once

#include "Goknar/Core.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "Goknar/Math/GoknarMath.h"

struct ImGuiContext;

class GOKNAR_API UIContext
{
public:
	static UIContext* Get()
	{
		if (!instance_)
		{
			instance_ = new UIContext();
		}

		return instance_;
	}

	static void Destroy();

	~UIContext();

	void Init();

	std::string objectToCreateName;
	std::string sceneSavePath;

	Vector2i windowSize;
	Vector2i buttonSize;

	ImGuiContext* imguiContext_{ nullptr };

private:
	UIContext();

	inline static UIContext* instance_{ nullptr };
};
