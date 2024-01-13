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
		extern const char* BONE_IDS;
		extern const char* WEIGHTS;
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
		extern const char* LIGHT_INTENSITY;
		extern const char* LIGHT_INTENSITY_PREFIX;

		extern const char* IS_CASTING_SHADOW;

		extern const char* DIRECTIONAL_LIGHT;
		extern const char* POINT_LIGHT;
		extern const char* SPOT_LIGHT;
	}

	inline namespace LIGHT_KEYWORDS
	{
		extern const char* POSITION;
		extern const char* INTENSITY;
		extern const char* RADIUS;
		extern const char* DIRECTION;
		extern const char* COVERAGE_ANGLE;
		extern const char* FALLOFF_ANGLE;

		extern const char* IS_CASTING_SHADOW;
		extern const char* CALCULATE_LIGHT_ADDITION;
	}

	inline namespace SHADOW
	{
		extern const char* FRAGMENT_TO_LIGHT_VECTOR_PREFIX;
		extern const char* SHADOW_MAP_PREFIX;
		extern const char* VIEW_MATRIX_PREFIX;

		extern const char* SHADOW_VALUE_PREFIX;

		extern const char* POINT_LIGHT_VIEW_MATRICES_ARRAY;

		extern const char* SHADOW_MAP_FRAGMENT_SHADER_POINT_LIGHT_POSITION;
		extern const char* SHADOW_MAP_FRAGMENT_SHADER_POINT_LIGHT_RADIUS;

		extern const char* LIGHT_POSITION;
		extern const char* LIGHT_RADIUS;
	}

	inline namespace SKELETAL_MESH
	{
		extern const char* BONES;
		extern const char* MAX_BONE_SIZE_MACRO;
	}

	inline namespace TEXTURE
	{
		extern const char* UV;

		extern const char* DIFFUSE;
		extern const char* NORMAL;
		extern const char* AMBIENT_OCCLUSION;
		extern const char* METALLIC;
		extern const char* SPECULAR;
		extern const char* ROUGHNESS;
	}

	inline namespace POSITIONING
	{
		extern const char* BONE_TRANSFORMATION_MATRIX;
		extern const char* WORLD_TRANSFORMATION_MATRIX;
		extern const char* RELATIVE_TRANSFORMATION_MATRIX;
		extern const char* MODEL_MATRIX;
		extern const char* VIEW_PROJECTION_MATRIX;
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
		extern const char* FINAL_MODEL_MATRIX;
		extern const char* FRAGMENT_POSITION_WORLD_SPACE;
		extern const char* FRAGMENT_POSITION_SCREEN_SPACE;
		extern const char* FRAGMENT_POSITION_LIGHT_SPACE_PREFIX;
		extern const char* VERTEX_NORMAL;
		extern const char* BONE_IDS;
		extern const char* WEIGHTS;
	}

	inline namespace GBUFFER
	{
		extern const char* OUT_POSITION;
		extern const char* OUT_NORMAL;
		extern const char* OUT_DIFFUSE;
		extern const char* OUT_SPECULAR_PHONG;
	}

	inline namespace FRAGMENT_SHADER_OUTS
	{
		extern const char* FRAGMENT_COLOR;
	}
}

#endif