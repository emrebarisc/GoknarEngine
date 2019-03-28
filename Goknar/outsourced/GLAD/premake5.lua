project "GLAD"
    kind "StaticLib"
    language "C"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files
	{
        "GLAD/include/glad/glad.h",
        "GLAD/include/KHR/khrplatform.h",
        "GLAD/src/glad.c"
    }

	includedirs
	{
		"GLAD/include"
	}
    
	filter "system:windows"
        systemversion "latest"
        staticruntime "On"

    filter { "system:windows", "configurations:Release" }
        buildoptions "/MT"
