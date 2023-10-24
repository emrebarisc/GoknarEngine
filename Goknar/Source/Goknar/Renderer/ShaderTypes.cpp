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
		const char* BONE_IDS = "boneIDs";
		const char* WEIGHTS = "weights";
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
		const char* SHADOW_MAP_POSTFIX = "ShadowMap";
		const char* VIEW_MATRIX_POSTFIX = "ViewMatrix";

		const char* DIRECTIONAL_LIGHT = "DirectionalLight";
		const char* POINT_LIGHT = "PointLight";
		const char* SPOT_LIGHT = "SpotLight";

		const char* SHADOW_CALCULATION_FUNCTION_NAME = "ShadowCalculation";
	}

	inline namespace LIGHT_KEYWORDS
	{
		const char* POSITION = "Position";
		const char* INTENSITY = "Intensity";
		const char* RADIUS = "Radius";
		const char* DIRECTION = "Direction";
		const char* COVERAGE_ANGLE = "CoverageAngle";
		const char* FALLOFF_ANGLE = "FalloffAngle";

		const char* IS_CASTING_SHADOW = "IsCastingShadow";
		const char* SHADOW_MAP = "ShadowMap";
	}

	inline namespace SKELETAL_MESH
	{
		const char* BONES = "bones";
		const char* MAX_BONE_SIZE_MACRO = "MAX_BONE_SIZE_PER_VERTEX";
	}

	inline namespace TEXTURE
	{
		const char* UV = "textureUV";

		const char* DIFFUSE = "diffuseTexture";
		const char* NORMAL = "normalTexture";
		const char* AMBIENT_OCCLUSION = "ambientOcclusionTexture";
		const char* METALLIC = "metallicTexture";
		const char* SPECULAR = "specularTexture";
		const char* ROUGHNESS = "roughnessTexture";
	}

	inline namespace POSITIONING
	{
		const char* BONE_TRANSFORMATION_MATRIX = "boneTransformationMatrix";
		const char* WORLD_TRANSFORMATION_MATRIX = "worldTransformationMatrix";
		const char* RELATIVE_TRANSFORMATION_MATRIX = "relativeTransformationMatrix";
		const char* MODEL_MATRIX = "modelMatrix";
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
		const char* FINAL_MODEL_MATRIX = "finalModelMatrix";
		const char* FRAGMENT_POSITION_WORLD_SPACE = "fragmentPositionWorldSpace";
		const char* FRAGMENT_POSITION_SCREEN_SPACE = "fragmentPositionScreenSpace";
		const char* VERTEX_NORMAL = "vertexNormal";
		const char* BONE_IDS = "outBoneIDs";
		const char* WEIGHTS = "outWeights";
	}

	inline namespace FRAGMENT_SHADER_OUTS
	{
		const char* FRAGMENT_COLOR = "fragmentColor";
	}
}
