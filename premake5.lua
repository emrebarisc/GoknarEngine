workspace "GameEngine"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Shipping"
	}

outputdir = "_%{cfg.buildcfg}_%{cfg.system}_%{cfg.architecture}"

project "GameEngine"
	location "GameEngine"
	kind "SharedLib"
	language "C++"

	targetdir("bin" .. outputdir .. "/%{prj.name}")
	objdir("obj" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/outsourced/spdlog/include"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"
		language "C++"
		staticruntime "off"

		defines
		{
			"GAMEENGINE_PLATFORM_WINDOWS",
			"GAMEENGINE_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin" .. outputdir .. "/GameProject")
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

	targetdir("bin" .. outputdir .. "/%{prj.name}")
	objdir("obj" .. outputdir .. "/%{prj.name}")

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
