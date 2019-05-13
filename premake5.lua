workspace "Goknar Engine"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Shipping"
	}

outputdir = "%{cfg.buildcfg}_%{cfg.system}_%{cfg.architecture}"

outsourceDirs = {}
outsourceDirs["GLFW"] = "Goknar/outsourced/GLFW/GLFW"
outsourceDirs["GLAD"] = "Goknar/outsourced/GLAD/GLAD"
outsourceDirs["ImGui"] = "Goknar/outsourced/ImGui/imgui"

includeDir = {}
includeDir["GLFW"] = "%{outsourceDirs.GLFW}/include"
includeDir["GLAD"] = "%{outsourceDirs.GLAD}/include"
includeDir["ImGui"] = "%{outsourceDirs.ImGui}"

include "Goknar/outsourced/GLFW"
include "Goknar/outsourced/GLAD"
include "Goknar/outsourced/ImGui"

project "Goknar"
	location "Goknar"
	kind "SharedLib"
	language "C++"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("obj/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "Goknar/src/pch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/outsourced/spdlog/include",
		"%{includeDir.GLFW}",
		"%{includeDir.GLAD}",
		"%{includeDir.ImGui}",
		"Goknar/src"
	}

	links
	{
		"GLFW",
		"GLAD",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"
		language "C++"
		staticruntime "off"

		defines
		{
			"GOKNAR_PLATFORM_WINDOWS",
			"GOKNAR_BUILD_DLL",
			"GOKNAR_ENABLE_ASSERTS",
			-- For GLFW not to include GLFW again
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../GameProject")
		}

	filter "configurations:Debug"
		defines "GOKNAR_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "GOKNAR_RELEASE"
		symbols "On"

	filter "configurations:Shipping"
		defines "GOKNAR_SHIPPING"
		symbols "On"

project "GameProject"
	location "GameProject"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("obj/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Goknar/outsourced/spdlog/include",
		"Goknar/src"
	}

	links
	{
		"Goknar"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines
		{
			"GOKNAR_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "GOKNAR_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "GOKNAR_RELEASE"
		runtime "Release"
		symbols "On"

	filter "configurations:Shipping"
		defines "GOKNAR_SHIPPING"
		runtime "Debug"
		symbols "On"
