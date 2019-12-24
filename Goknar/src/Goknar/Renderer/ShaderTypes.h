#ifndef __SHADERTYPES_H__
#define __SHADERTYPES_H__

extern const char* DEFAULT_SHADER_VERSION;

inline namespace SHADER_VARIABLE_NAMES
{
	inline namespace MATERIAL
	{
		extern const char* AMBIENT;
		extern const char* DIFFUSE;
		extern const char* SPECULAR;
		extern const char* PHONG_EXPONENT;
	}

	inline namespace LIGHT
	{
		extern const char* DIRECTIONAL_LIGHT;
		extern const char* POINT_LIGHT;
		extern const char* SPOT_LIGHT;
	}

	inline namespace LIGHT_KEYWORDS
	{
		extern const char* POSITION;
		extern const char* INTENSITY;
		extern const char* DIRECTION;
		extern const char* COVERAGE_ANGLE;
		extern const char* FALLOFF_ANGLE;
	}

	inline namespace TEXTURE
	{
		extern const char* DIFFUSE;
	}
}

#endif