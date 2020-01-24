#include "pch.h"

#include "ShaderTypes.h"

const char* DEFAULT_SHADER_VERSION = "440 core";

inline namespace SHADER_VARIABLE_NAMES
{
	inline namespace VERTEX
	{
		const char* POSITION = "position";
		const char* NORMAL = "normal";
		const char* COLOR = "color";
		const char* UV = "uv";
	}

	inline namespace MATERIAL
	{
		const char* AMBIENT = "ambientReflectance";
		const char* DIFFUSE = "diffuseReflectance";
		const char* SPECULAR = "specularReflectance";
		const char* PHONG_EXPONENT = "phongExponent";
	}

	inline namespace LIGHT
	{
		const char* DIRECTIONAL_LIGHT = "DirectionalLight";
		const char* POINT_LIGHT = "PointLight";
		const char* SPOT_LIGHT = "SpotLight";
	}

	inline namespace LIGHT_KEYWORDS
	{
		const char* POSITION = "Position";
		const char* INTENSITY = "Intensity";
		const char* DIRECTION = "Direction";
		const char* COVERAGE_ANGLE = "CoverageAngle";
		const char* FALLOFF_ANGLE = "FalloffAngle";
	}

	inline namespace TEXTURE
	{
		const char* DIFFUSE = "diffuseTexture";
		const char* NORMAL = "normalTexture";
		const char* AMBIENT_OCCLUSION = "ambientOcclusionTexture";
		const char* METALLIC = "metallicTexture";
		const char* SPECULAR = "specularTexture";
		const char* ROUGHNESS = "roughnessTexture";
	}

	inline namespace POSITIONING
	{
		const char* MODEL_MATRIX = "transformationMatrix";
		const char* VIEW_MATRIX = "viewMatrix";
		const char* PROJECTION_MATRIX = "projectionMatrix";
		const char* TRANSFORMATION_MATRIX = "transformationMatrix";
		const char* VIEW_POSITION = "viewPosition";
	}

	inline namespace TIMING
	{
		const char* DELTA_TIME = "deltaTime";
		const char* ELAPSED_TIME = "elapsedTime";
	}

	inline namespace VERTEX_SHADER_OUTS
	{
		const char* FRAGMENT_POSITION = "fragmentPosition";
		const char* VERTEX_NORMAL = "vertexNormal";
	}

	inline namespace FRAGMENT_SHADER_OUTS
	{
		const char* FRAGMENT_COLOR = "fragmentColor";
	}
}
