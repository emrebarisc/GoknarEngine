#pragma once

#include <string>

#include "imgui.h"

#include "Goknar/Math/GoknarMath.h"

class Vector3;
class Quaternion;

struct Folder;

class UIWidgets
{
public:
	static void DrawInputText(const std::string& name, std::string& value);
	static void DrawInputInt(const std::string& name, int& value);
	static void DrawInputFloat(const std::string& name, float& value);
	static void DrawInputVector3(const std::string& name, Vector3& vector);
	static void DrawInputQuaternion(const  std::string& name, Quaternion& quaternion);
	static void DrawCheckbox(const std::string& name, bool& value);

	static bool DrawWindowWithOneTextBoxOneButton(
		const std::string& windowTitle,
		const std::string& text,
		const std::string& currentValue,
		const std::string& buttonText,
		const Vector2i& size,
		std::string& resultText,
		bool& isOpen,
		ImGuiWindowFlags flags);


	static void DrawFileGrid(const Folder* folder, std::string& selectedFileName, bool& isAFileSelected);
};