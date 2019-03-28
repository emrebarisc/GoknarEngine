workspace "GameEngine"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Shipping"
	}

outputdir = "%{cfg.buildcfg}_%{cfg.system}_%{cfg.architecture}"

outsourceDirs = {}
outsourceDirs["GLFW"] = "GameEngine/outsourced/GLFW"

includeDir = {}
includeDir["GLFW"] = "GameEngine/outsourced/GLFW/include"

--include "GameEngine/outsourced/GLFW"

project "GameEngine"
	location "GameEngine"
	kind "SharedLib"
	language "C++"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("obj/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "GameEngine/src/GameEngine/pch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/outsourced/spdlog/include",
		"%{includeDir.GLFW}",
		"GameEngine/src"
	}

	links
	{
		"GLFW",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"
		language "C++"
		staticruntime "off"

		defines
		{
			"GAMEENGINE_PLATFORM_WINDOWS",
			"GAMEENGINE_BUILD_DLL",

			"ENGINE_ENABLE_ASSERTS"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../GameProject")
		}

	filter "configurations:Debug"
		defines "GAMEENGINE_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "GAMEENGINE_RELEASE"
		symbols "On"

	filter "configurations:Shipping"
		defines "GAMEENGINE_SHIPPING"
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
		"GameEngine/outsourced/spdlog/include",
		"GameEngine/src"
	}

	links
	{
		"GameEngine"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines
		{
			"GAMEENGINE_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "GAMEENGINE_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "GAMEENGINE_RELEASE"
		runtime "Release"
		symbols "On"

	filter "configurations:Shipping"
		defines "GAMEENGINE_SHIPPING"
		runtime "Debug"
		symbols "On"

project "GLFW"
    kind "StaticLib"
    language "C"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files
	{
        "%{outsourceDirs.GLFW}/include/GLFW/glfw3.h",
        "%{outsourceDirs.GLFW}/include/GLFW/glfw3native.h",
        "%{outsourceDirs.GLFW}/src/glfw_config.h",
        "%{outsourceDirs.GLFW}/src/context.c",
        "%{outsourceDirs.GLFW}/src/init.c",
        "%{outsourceDirs.GLFW}/src/input.c",
        "%{outsourceDirs.GLFW}/src/monitor.c",
        "%{outsourceDirs.GLFW}/src/vulkan.c",
        "%{outsourceDirs.GLFW}/src/window.c"
    }
    
	filter "system:windows"
        buildoptions { "-std=c11", "-lgdi32" }
        systemversion "latest"
        staticruntime "On"
        
        files
        {
            "%{outsourceDirs.GLFW}/src/win32_init.c",
            "%{outsourceDirs.GLFW}/src/win32_joystick.c",
            "%{outsourceDirs.GLFW}/src/win32_monitor.c",
            "%{outsourceDirs.GLFW}/src/win32_time.c",
            "%{outsourceDirs.GLFW}/src/win32_thread.c",
            "%{outsourceDirs.GLFW}/src/win32_window.c",
            "%{outsourceDirs.GLFW}/src/wgl_context.c",
            "%{outsourceDirs.GLFW}/src/egl_context.c",
            "%{outsourceDirs.GLFW}/src/osmesa_context.c"
        }

		defines 
		{ 
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"
		}
    filter { "system:windows", "configurations:Release" }
        buildoptions "/MT"
