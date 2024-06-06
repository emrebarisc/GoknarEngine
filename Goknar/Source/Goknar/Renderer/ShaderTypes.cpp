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
		const char* PHONG_EXPONENT = "phongExponent";
		const char* TRANSLUCENCY = "translucency";

		const char* BASE_COLOR = "baseColor";
		const char* METALLIC = "metallic";
		const char* SPECULAR = "specular";
		const char* ROUGHNESS = "roughness";
		const char* EMMISIVE_COLOR = "emmisiveColor";
		const char* AMBIENT_OCCLUSION = "ambientOcclusion";
	}

	inline namespace LIGHT
	{
		const char* LIGHT_INTENSITY = "lightIntensity";
		const char* LIGHT_INTENSITY_PREFIX = "lightIntensity_";
		const char* CURRENT_LIGHT_INTENSITY = "currentLightIntensity";

		const char* DIRECTIONAL_LIGHT = "DirectionalLight";
		const char* POINT_LIGHT = "PointLight";
		const char* SPOT_LIGHT = "SpotLight";

		const char* DIRECTIONAL_LIGHT_UNIFORM_NAME = "DirectionalLightUniform";
		const char* POINT_LIGHT_UNIFORM_NAME = "PointLightUniform";
		const char* SPOT_LIGHT_UNIFORM_NAME = "SpotLightUniform";

		const char* DIRECTIONAL_LIGHT_STRUCT_NAME = "DirectionalLight";
		const char* POINT_LIGHT_STRUCT_NAME = "PointLight";
		const char* SPOT_LIGHT_STRUCT_NAME = "SpotLight";
		
		const char* DIRECTIONAL_LIGHT_SHADOW_MAP_ARRAY_NAME = "directionalLightShadowMaps";
		const char* POINT_LIGHT_SHADOW_MAP_ARRAY_NAME = "pointLightShadowMaps";
		const char* SPOT_LIGHT_SHADOW_MAP_ARRAY_NAME = "spotLightShadowMaps";

		const char* MAX_DIRECTIONAL_LIGHT_COUNT_MACRO = "MAX_DIRECTIONAL_LIGHT_COUNT";
		const char* MAX_POINT_LIGHT_COUNT_MACRO = "MAX_POINT_LIGHT_COUNT";
		const char* MAX_SPOT_LIGHT_COUNT_MACRO = "MAX_SPOT_LIGHT_COUNT";

		const char* DIRECTIONAL_LIGHT_COUNT_IN_USE_VARIABLE = "directionalLightCount";
		const char* POINT_LIGHT_COUNT_IN_USE_VARIABLE = "pointLightCount";
		const char* SPOT_LIGHT_COUNT_IN_USE_VARIABLE = "spotLightCount";

		const char* DIRECTIONAL_LIGHT_ARRAY_NAME = "directionalLights";
		const char* POINT_LIGHT_ARRAY_NAME = "pointLights";
		const char* SPOT_LIGHT_ARRAY_NAME = "spotLights";

		const char* DIRECTIONAL_LIGHT_VIEW_MATRIX_UNIFORM_NAME = "DirectionalLightViewMatrixUniformBuffer";
		const char* POINT_LIGHT_VIEW_MATRIX_UNIFORM_NAME = "PointLightViewMatrixUniformBuffer";
		const char* SPOT_LIGHT_VIEW_MATRIX_UNIFORM_NAME = "SpotLightViewMatrixUniformBuffer";

		const char* DIRECTIONAL_LIGHT_VIEW_MATRIX_ARRAY_NAME = "directionalLightViewMatrixArray";
		const char* POINT_LIGHT_VIEW_MATRIX_ARRAY_NAME = "pointLightViewMatrixArray";
		const char* SPOT_LIGHT_VIEW_MATRIX_ARRAY_NAME = "spotLightViewMatrixArray";
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
		const char* CALCULATE_LIGHT_ADDITION = "CalculateFullLightAddition";
	}

	inline namespace SHADOW
	{
		const char* FRAGMENT_TO_LIGHT_VECTOR = "fragmentToLightVector";
		const char* FRAGMENT_TO_LIGHT_VECTOR_PREFIX = "fragmentToLightVector_";
		const char* SHADOW_MAP = "shadowMap";
		const char* SHADOW_MAP_PREFIX = "shadowMap_";
		const char* VIEW_MATRIX_PREFIX = "lightViewMatrix_";
		const char* SHADOW_VALUE = "shadowValue";
		const char* SHADOW_VALUE_PREFIX = "shadowValue_";

		const char* POINT_LIGHT_VIEW_MATRICES_ARRAY = "viewMatrices";

		const char* SHADOW_MAP_FRAGMENT_SHADER_POINT_LIGHT_POSITION = "pointLightPosition";
		const char* SHADOW_MAP_FRAGMENT_SHADER_POINT_LIGHT_RADIUS = "pointLightRadius";

		const char* LIGHT_POSITION = "lightPosition";
		const char* LIGHT_RADIUS = "lightRadius";
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
		const char* VIEW_PROJECTION_MATRIX = "viewProjectionMatrix";
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
		const char* FRAGMENT_POSITION_LIGHT_SPACE_PREFIX = "fragmentPositionLightSpace_";
		const char* VERTEX_NORMAL = "vertexNormal";
		const char* BONE_IDS = "outBoneIDs";
		const char* WEIGHTS = "outWeights";

		const char* DIRECTIONAL_LIGHT_SPACE_FRAGMENT_POSITIONS = "directionalLightSpaceFragmentPositions";
		const char* POINT_LIGHT_SPACE_FRAGMENT_POSITIONS = "pointLightSpaceFragmentPositions";
		const char* SPOT_LIGHT_SPACE_FRAGMENT_POSITIONS = "spotLightSpaceFragmentPositions";
	}

	inline namespace GBUFFER
	{
		const char* OUT_POSITION = "position_GBuffer";
		const char* OUT_NORMAL = "normal_GBuffer";
		const char* OUT_DIFFUSE = "diffuse_GBuffer";
		const char* OUT_SPECULAR_PHONG = "specularAndPhong_GBuffer";
	}

	inline namespace FRAGMENT_SHADER_OUTS
	{
		const char* FRAGMENT_COLOR = "fragmentColor";
	}

	inline namespace CALCULATIONS
	{
		const char* FINAL_BASE_COLOR = "finalBaseColor";
	}
}
