#include "UIWidgets.h"

#include "imgui.h"

#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/Quaternion.h"

#include "UIUtils.h"

void UIWidgets::DrawInputText(const std::string& name, std::string& value)
{
	char* valueChar = const_cast<char*>(value.c_str());
	ImGui::InputText(name.c_str(), valueChar, 64);
	value = std::string(valueChar);
}

void UIWidgets::DrawInputInt(const std::string& name, int& value)
{
	int newValue = value;
	ImGui::InputInt((name).c_str(), &newValue);
	value = newValue;
}

void UIWidgets::DrawInputFloat(const std::string& name, float& value)
{
	float newValue = value;
	ImGui::InputFloat((name).c_str(), &newValue);
	if (SMALLER_EPSILON < GoknarMath::Abs(value - newValue))
	{
		value = newValue;
	}
}

void UIWidgets::DrawInputVector3(const std::string& name, Vector3& vector)
{
	float newValueX = vector.x;
	ImGui::InputFloat((name + "X").c_str(), &newValueX);
	if (SMALLER_EPSILON < GoknarMath::Abs(vector.x - newValueX))
	{
		vector.x = newValueX;
	}

	ImGui::SameLine();

	float newValueY = vector.y;
	ImGui::InputFloat((name + "Y").c_str(), &newValueY);
	if (SMALLER_EPSILON < GoknarMath::Abs(vector.y - newValueY))
	{
		vector.y = newValueY;
	}

	ImGui::SameLine();

	float newValueZ = vector.z;
	ImGui::InputFloat((name + "Z").c_str(), &newValueZ);
	if (SMALLER_EPSILON < GoknarMath::Abs(vector.z - newValueZ))
	{
		vector.z = newValueZ;
	}
}

void UIWidgets::DrawInputQuaternion(const  std::string& name, Quaternion& quaternion)
{
	float newQuaternionX = quaternion.x;
	ImGui::InputFloat((name + "X").c_str(), &newQuaternionX);
	if (SMALLER_EPSILON < GoknarMath::Abs(quaternion.x - newQuaternionX))
	{
		quaternion.x = newQuaternionX;
	}

	ImGui::SameLine();

	float newQuaternionY = quaternion.y;
	ImGui::InputFloat((name + "Y").c_str(), &newQuaternionY);
	if (SMALLER_EPSILON < GoknarMath::Abs(quaternion.y - newQuaternionY))
	{
		quaternion.y = newQuaternionY;
	}

	ImGui::SameLine();

	float newQuaternionZ = quaternion.z;
	ImGui::InputFloat((name + "Z").c_str(), &newQuaternionZ);
	if (SMALLER_EPSILON < GoknarMath::Abs(quaternion.z - newQuaternionZ))
	{
		quaternion.z = newQuaternionZ;
	}

	ImGui::SameLine();

	float newQuaternionW = quaternion.w;
	ImGui::InputFloat((name + "W").c_str(), &newQuaternionW, 64);
	if (SMALLER_EPSILON < GoknarMath::Abs(quaternion.w - newQuaternionW))
	{
		quaternion.w = newQuaternionW;
	}
}

void UIWidgets::DrawCheckbox(const std::string& name, bool& value)
{
	ImGui::Checkbox(name.c_str(), &value);
}

bool UIWidgets::DrawWindowWithOneTextBoxOneButton(const std::string& windowTitle, const std::string& text, const std::string& currentValue, const std::string& buttonText, const Vector2i& size, std::string& resultText, bool& isOpen, ImGuiWindowFlags flags)
{
	ImGui::SetNextWindowPos(UIUtils::ToImVec2((UIContext::Get()->windowSize - size) * 0.5f));
	ImGui::SetNextWindowSize(UIUtils::ToImVec2(size));

	flags |= ImGuiWindowFlags_AlwaysAutoResize;

	ImGui::Begin(windowTitle.c_str(), &isOpen, flags);

	ImGui::Text(text.c_str());
	ImGui::SameLine();

	char* input = const_cast<char*>(currentValue.c_str());
	ImGui::InputText((std::string("##") + windowTitle + "_TextBox").c_str(), input, 1024);

	std::string inputString = input;
	bool buttonClicked = ImGui::Button(buttonText.c_str());
	buttonClicked = buttonClicked && !inputString.empty();

	if (buttonClicked)
	{
		resultText = inputString;
	}

	ImGui::End();

	return buttonClicked;
}
