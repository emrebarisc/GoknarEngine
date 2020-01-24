#ifndef __SHADERTYPES_H__
#define __SHADERTYPES_H__

extern const char* DEFAULT_SHADER_VERSION;

inline namespace SHADER_VARIABLE_NAMES
{
	inline namespace VERTEX
	{
		extern const char* POSITION;
		extern const char* NORMAL;
		extern const char* COLOR;
		extern const char* UV;
	}

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

	inline namespace POSITIONING
	{
		extern const char* MODEL_MATRIX;
		extern const char* VIEW_MATRIX;
		extern const char* PROJECTION_MATRIX;
		extern const char* TRANSFORMATION_MATRIX;
		extern const char* VIEW_POSITION;
	}

	inline namespace TIMING
	{
		extern const char* DELTA_TIME;
		extern const char* ELAPSED_TIME;
	}

	inline namespace VERTEX_SHADER_OUTS
	{
		extern const char* FRAGMENT_POSITION;
		extern const char* VERTEX_NORMAL;
	}

	inline namespace FRAGMENT_SHADER_OUTS
	{
		extern const char* FRAGMENT_COLOR;
	}
}

#endif