// Deferred Rendering Pass Fragment Shader
#version 440 core

out vec3 fragmentColor;

in vec2 textureUV;
uniform sampler2D position_GBuffer;
uniform sampler2D normal_GBuffer;
uniform sampler2D diffuse_GBuffer;
uniform sampler2D specularAndPhong_GBuffer;

//---------------------------------------- SHADOW MAPS ----------------------------------------
uniform mat4 lightViewMatrix_DirectionalLight0;
uniform mat4 lightViewMatrix_SpotLight4;
uniform mat4 lightViewMatrix_SpotLight0;
uniform mat4 lightViewMatrix_SpotLight1;
uniform mat4 lightViewMatrix_SpotLight2;
uniform mat4 lightViewMatrix_SpotLight3;
//----------------------------------------------------------------------------------------------


//---------------------------------------- SHADOW MAPS ----------------------------------------
uniform sampler2DShadow shadowMap_DirectionalLight0;
uniform sampler2DShadow shadowMap_SpotLight4;
uniform sampler2DShadow shadowMap_SpotLight0;
uniform sampler2DShadow shadowMap_SpotLight1;
uniform sampler2DShadow shadowMap_SpotLight2;
uniform sampler2DShadow shadowMap_SpotLight3;
uniform samplerCubeShadow shadowMap_PointLight16;
//----------------------------------------------------------------------------------------------

uniform vec3 PointLight16Position;
uniform vec3 PointLight16Intensity;
uniform float PointLight16Radius;
uniform vec3 DirectionalLight0Direction;
uniform vec3 DirectionalLight0Intensity;

uniform vec3 SpotLight0Position;
uniform vec3 SpotLight0Direction;
uniform vec3 SpotLight0Intensity;
uniform float SpotLight0CoverageAngle;
uniform float SpotLight0FalloffAngle;
uniform vec3 SpotLight1Position;
uniform vec3 SpotLight1Direction;
uniform vec3 SpotLight1Intensity;
uniform float SpotLight1CoverageAngle;
uniform float SpotLight1FalloffAngle;
uniform vec3 SpotLight2Position;
uniform vec3 SpotLight2Direction;
uniform vec3 SpotLight2Intensity;
uniform float SpotLight2CoverageAngle;
uniform float SpotLight2FalloffAngle;
uniform vec3 SpotLight3Position;
uniform vec3 SpotLight3Direction;
uniform vec3 SpotLight3Intensity;
uniform float SpotLight3CoverageAngle;
uniform float SpotLight3FalloffAngle;
vec3 PointLight0Position = vec3(-20.000000, -20.000000, 3.000000);
vec3 PointLight0Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight0Radius = 25.000000;
vec3 PointLight1Position = vec3(-25.000000, -20.000000, 3.000000);
vec3 PointLight1Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight1Radius = 25.000000;
vec3 PointLight2Position = vec3(-30.000000, -20.000000, 3.000000);
vec3 PointLight2Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight2Radius = 25.000000;
vec3 PointLight3Position = vec3(-35.000000, -20.000000, 3.000000);
vec3 PointLight3Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight3Radius = 25.000000;
vec3 PointLight4Position = vec3(-20.000000, -25.000000, 3.000000);
vec3 PointLight4Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight4Radius = 25.000000;
vec3 PointLight5Position = vec3(-25.000000, -25.000000, 3.000000);
vec3 PointLight5Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight5Radius = 25.000000;
vec3 PointLight6Position = vec3(-30.000000, -25.000000, 3.000000);
vec3 PointLight6Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight6Radius = 25.000000;
vec3 PointLight7Position = vec3(-35.000000, -25.000000, 3.000000);
vec3 PointLight7Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight7Radius = 25.000000;
vec3 PointLight8Position = vec3(-20.000000, -30.000000, 3.000000);
vec3 PointLight8Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight8Radius = 25.000000;
vec3 PointLight9Position = vec3(-25.000000, -30.000000, 3.000000);
vec3 PointLight9Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight9Radius = 25.000000;
vec3 PointLight10Position = vec3(-30.000000, -30.000000, 3.000000);
vec3 PointLight10Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight10Radius = 25.000000;
vec3 PointLight11Position = vec3(-35.000000, -30.000000, 3.000000);
vec3 PointLight11Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight11Radius = 25.000000;
vec3 PointLight12Position = vec3(-20.000000, -35.000000, 3.000000);
vec3 PointLight12Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight12Radius = 25.000000;
vec3 PointLight13Position = vec3(-25.000000, -35.000000, 3.000000);
vec3 PointLight13Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight13Radius = 25.000000;
vec3 PointLight14Position = vec3(-30.000000, -35.000000, 3.000000);
vec3 PointLight14Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight14Radius = 25.000000;
vec3 PointLight15Position = vec3(-35.000000, -35.000000, 3.000000);
vec3 PointLight15Intensity = vec3(10.000000, 10.000000, 10.000000);
float PointLight15Radius = 25.000000;
vec3 SpotLight4Position = vec3(50.000000, 30.000000, 10.000000);
vec3 SpotLight4Direction = vec3(-0.577350, 0.577350, -0.577350);
vec3 SpotLight4Intensity = vec3(100.000000, 75.000000, 75.000000);
float SpotLight4CoverageAngle = 0.349066;
float SpotLight4FalloffAngle = 0.087266;
uniform vec3 viewPosition;
uniform bool isDebugging;

vec4 fragmentPositionWorldSpace;
vec3 vertexNormal;
vec3 diffuseReflectance;
vec3 specularReflectance;
float phongExponent;
vec3 sceneAmbient = vec3(0.039216, 0.039216, 0.039216);

vec3 CalculatePointLightColor(vec3 position, vec3 intensity, float radius)
{
	// To light vector
	vec3 wi = position - vec3(fragmentPositionWorldSpace);
	float wiLength = length(wi);

	if(radius < wiLength)
	{
		return vec3(0.f);
	}

	wi /= wiLength;

	if(dot(vertexNormal, wi) < 0.f) return vec3(0.f, 0.f, 0.f);

	// To viewpoint vector
	vec3 wo = viewPosition - vec3(fragmentPositionWorldSpace);
	float woLength = length(wo);
	wo /= woLength;

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;

	float inverseDistanceSquare = 1 / (wiLength * wiLength);

	// Diffuse
	float cosThetaPrime = max(0.f, dot(wi, vertexNormal));
	vec3 color = vec3(0.f);

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	color += specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	color *= cosThetaPrime * intensity * inverseDistanceSquare;

	return clamp(color, 0.f, 1.f);
}


vec3 CalculateDirectionalLightColor(vec3 direction, vec3 intensity)
{
	vec3 wi = -direction;
	float wiLength = length(wi);
	wi /= wiLength;

	if(dot(vertexNormal, wi) < 0.f) return vec3(0.f, 0.f, 0.f);

	float normalDotLightDirection = dot(vertexNormal, wi);

	vec3 color = vec3(0.f);

	// To viewpoint vector
	vec3 wo = viewPosition - vec3(fragmentPositionWorldSpace);
	float woLength = length(wo);
	wo /= woLength;

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;
	float inverseDistanceSquare = 1 / (wiLength * wiLength);

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	color += specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	color *= max(0, normalDotLightDirection) * intensity * inverseDistanceSquare;

	return clamp(color, 0.f, 1.f);
}


vec3 CalculateSpotLightColor(vec3 position, vec3 direction, vec3 intensity, float coverageAngle, float falloffAngle)
{
	float lightMultiplier = 0.f;
	vec3 specular = vec3(0.f, 0.f, 0.f);

	// To light vector
	vec3 wi = vec3(fragmentPositionWorldSpace) - position;
	float wiLength = length(wi);
	wi /= wiLength;

	if(0.f < dot(wi, vertexNormal)) return vec3(0.f);

	float inverseDistanceSquare = 1.f / (wiLength * wiLength);

	float cosCoverage = cos(coverageAngle);
	float cosFalloff = cos(falloffAngle);
	float cosTheta = dot(wi, direction);
	
	if(cosTheta < cosCoverage)
	{
		return vec3(0.f);
	}

	if(cosFalloff < cosTheta)
	{
		lightMultiplier = 1.f;
	}
	else
	{
		lightMultiplier = pow((cosTheta - cosCoverage) / (cosFalloff - cosCoverage), 8);
	}

	// To viewpoint vector
	vec3 wo = normalize(viewPosition - vec3(fragmentPositionWorldSpace));

	// Half vector
	vec3 halfVector = (-wi + wo) * 0.5f;

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	specular = specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	vec3 color = (1.f + specular) * lightMultiplier * intensity * inverseDistanceSquare;
	return clamp(color, 0.f, 1.f);
}


void main()
{
	if(isDebugging)
	{
		if(textureUV.x < 0.5f)
		{
			if(textureUV.y < 0.5f)
			{
				fragmentColor = texture(diffuse_GBuffer, textureUV * 2.f).xyz;
			}
			else
			{
				fragmentColor = texture(position_GBuffer, (textureUV - vec2(0.f, 0.5f)) * 2.f).xyz;
			}
		}
		else
		{
			if(textureUV.y < 0.5f)
			{
				vec4 textureColor = texture(specularAndPhong_GBuffer, (textureUV - vec2(0.5f, 0.5f)) * 2.f);
				fragmentColor = vec3((textureColor.r + textureColor.g + textureColor.b + textureColor.a) * 0.25f);
			}
			else
			{
				fragmentColor = texture(normal_GBuffer, (textureUV - vec2(0.5f, 0.5f)) * 2.f).xyz * 0.5f + vec3(0.5f);
			}
		}

		return;
	}

	diffuseReflectance = texture(diffuse_GBuffer, textureUV).xyz;
	
	fragmentPositionWorldSpace = vec4(texture(position_GBuffer, textureUV).xyz, 1.f);
	vertexNormal = texture(normal_GBuffer, textureUV).xyz;
	
	vec4 specularAndPhongTextureValue = texture(specularAndPhong_GBuffer, textureUV);
	specularReflectance = specularAndPhongTextureValue.xyz;
	phongExponent = specularAndPhongTextureValue.a;
//------------------------ FRAGMENT POSITION LIGHT SPACE ------------------------

	 vec4 fragmentPositionLightSpace_DirectionalLight0 = fragmentPositionWorldSpace * lightViewMatrix_DirectionalLight0; 
	 vec4 fragmentPositionLightSpace_SpotLight4 = fragmentPositionWorldSpace * lightViewMatrix_SpotLight4; 
	 vec4 fragmentPositionLightSpace_SpotLight0 = fragmentPositionWorldSpace * lightViewMatrix_SpotLight0; 
	 vec4 fragmentPositionLightSpace_SpotLight1 = fragmentPositionWorldSpace * lightViewMatrix_SpotLight1; 
	 vec4 fragmentPositionLightSpace_SpotLight2 = fragmentPositionWorldSpace * lightViewMatrix_SpotLight2; 
	 vec4 fragmentPositionLightSpace_SpotLight3 = fragmentPositionWorldSpace * lightViewMatrix_SpotLight3; 
//-------------------------------------------------------------------------------

	vec3 lightColor = sceneAmbient;
	lightColor += CalculatePointLightColor(PointLight0Position, PointLight0Intensity, PointLight0Radius);
	lightColor += CalculatePointLightColor(PointLight1Position, PointLight1Intensity, PointLight1Radius);
	lightColor += CalculatePointLightColor(PointLight2Position, PointLight2Intensity, PointLight2Radius);
	lightColor += CalculatePointLightColor(PointLight3Position, PointLight3Intensity, PointLight3Radius);
	lightColor += CalculatePointLightColor(PointLight4Position, PointLight4Intensity, PointLight4Radius);
	lightColor += CalculatePointLightColor(PointLight5Position, PointLight5Intensity, PointLight5Radius);
	lightColor += CalculatePointLightColor(PointLight6Position, PointLight6Intensity, PointLight6Radius);
	lightColor += CalculatePointLightColor(PointLight7Position, PointLight7Intensity, PointLight7Radius);
	lightColor += CalculatePointLightColor(PointLight8Position, PointLight8Intensity, PointLight8Radius);
	lightColor += CalculatePointLightColor(PointLight9Position, PointLight9Intensity, PointLight9Radius);
	lightColor += CalculatePointLightColor(PointLight10Position, PointLight10Intensity, PointLight10Radius);
	lightColor += CalculatePointLightColor(PointLight11Position, PointLight11Intensity, PointLight11Radius);
	lightColor += CalculatePointLightColor(PointLight12Position, PointLight12Intensity, PointLight12Radius);
	lightColor += CalculatePointLightColor(PointLight13Position, PointLight13Intensity, PointLight13Radius);
	lightColor += CalculatePointLightColor(PointLight14Position, PointLight14Intensity, PointLight14Radius);
	lightColor += CalculatePointLightColor(PointLight15Position, PointLight15Intensity, PointLight15Radius);

	vec3 fragmentToLightVector_PointLight16 = fragmentPositionWorldSpace.xyz + vertexNormal * 0.025f - PointLight16Position;
	float shadowValue_PointLight16 = texture(shadowMap_PointLight16, vec4(fragmentToLightVector_PointLight16, 0.15f)).x;
	shadowValue_PointLight16 *= PointLight16Radius;
	if(length(fragmentToLightVector_PointLight16) < shadowValue_PointLight16)
	{
		vec3 lightColor_PointLight16 = CalculatePointLightColor(PointLight16Position, PointLight16Intensity, PointLight16Radius);
		lightColor += shadowValue_PointLight16 * lightColor_PointLight16;
	}

	vec3 lightSpaceScreenCoordinate_DirectionalLight0 = fragmentPositionLightSpace_DirectionalLight0.xyz / fragmentPositionLightSpace_DirectionalLight0.w;

	if(	0.f <= lightSpaceScreenCoordinate_DirectionalLight0.x && lightSpaceScreenCoordinate_DirectionalLight0.x <= 1.f &&
		0.f <= lightSpaceScreenCoordinate_DirectionalLight0.y && lightSpaceScreenCoordinate_DirectionalLight0.y <= 1.f)
	{
		float shadowValue_DirectionalLight0 = textureProj(shadowMap_DirectionalLight0, fragmentPositionLightSpace_DirectionalLight0);

		if(0.f < shadowValue_DirectionalLight0)
		{
			vec3 lightColor_DirectionalLight0 = CalculateDirectionalLightColor(DirectionalLight0Direction, DirectionalLight0Intensity); 
			lightColor += shadowValue_DirectionalLight0 * lightColor_DirectionalLight0;
		}
	}
	else
	{
		lightColor +=  CalculateDirectionalLightColor(DirectionalLight0Direction, DirectionalLight0Intensity);
	}

	vec3 lightSpaceScreenCoordinate_SpotLight4 = fragmentPositionLightSpace_SpotLight4.xyz / fragmentPositionLightSpace_SpotLight4.w;

	if(	0.f <= lightSpaceScreenCoordinate_SpotLight4.x && lightSpaceScreenCoordinate_SpotLight4.x <= 1.f &&
		0.f <= lightSpaceScreenCoordinate_SpotLight4.y && lightSpaceScreenCoordinate_SpotLight4.y <= 1.f)
	{
		float shadowValue_SpotLight4 = textureProj(shadowMap_SpotLight4, fragmentPositionLightSpace_SpotLight4);
		
		if(0.f < shadowValue_SpotLight4)
		{
			vec3 lightColor_SpotLight4 = CalculateSpotLightColor(SpotLight4Position, SpotLight4Direction, SpotLight4Intensity, SpotLight4CoverageAngle, SpotLight4FalloffAngle); 
			lightColor += shadowValue_SpotLight4 * lightColor_SpotLight4;
		}
	}
	else
	{
		lightColor +=  CalculateSpotLightColor(SpotLight4Position, SpotLight4Direction, SpotLight4Intensity, SpotLight4CoverageAngle, SpotLight4FalloffAngle);
	}

	vec3 lightSpaceScreenCoordinate_SpotLight0 = fragmentPositionLightSpace_SpotLight0.xyz / fragmentPositionLightSpace_SpotLight0.w;

	if(	0.f <= lightSpaceScreenCoordinate_SpotLight0.x && lightSpaceScreenCoordinate_SpotLight0.x <= 1.f &&
		0.f <= lightSpaceScreenCoordinate_SpotLight0.y && lightSpaceScreenCoordinate_SpotLight0.y <= 1.f)
	{
		float shadowValue_SpotLight0 = textureProj(shadowMap_SpotLight0, fragmentPositionLightSpace_SpotLight0);
		
		if(0.f < shadowValue_SpotLight0)
		{
			vec3 lightColor_SpotLight0 = CalculateSpotLightColor(SpotLight0Position, SpotLight0Direction, SpotLight0Intensity, SpotLight0CoverageAngle, SpotLight0FalloffAngle); 
			lightColor += shadowValue_SpotLight0 * lightColor_SpotLight0;
		}
	}
	else
	{
		lightColor +=  CalculateSpotLightColor(SpotLight0Position, SpotLight0Direction, SpotLight0Intensity, SpotLight0CoverageAngle, SpotLight0FalloffAngle);
	}

	vec3 lightSpaceScreenCoordinate_SpotLight1 = fragmentPositionLightSpace_SpotLight1.xyz / fragmentPositionLightSpace_SpotLight1.w;

	if(	0.f <= lightSpaceScreenCoordinate_SpotLight1.x && lightSpaceScreenCoordinate_SpotLight1.x <= 1.f &&
		0.f <= lightSpaceScreenCoordinate_SpotLight1.y && lightSpaceScreenCoordinate_SpotLight1.y <= 1.f)
	{
		float shadowValue_SpotLight1 = textureProj(shadowMap_SpotLight1, fragmentPositionLightSpace_SpotLight1);
		
		if(0.f < shadowValue_SpotLight1)
		{
			vec3 lightColor_SpotLight1 = CalculateSpotLightColor(SpotLight1Position, SpotLight1Direction, SpotLight1Intensity, SpotLight1CoverageAngle, SpotLight1FalloffAngle); 
			lightColor += shadowValue_SpotLight1 * lightColor_SpotLight1;
		}
	}
	else
	{
		lightColor +=  CalculateSpotLightColor(SpotLight1Position, SpotLight1Direction, SpotLight1Intensity, SpotLight1CoverageAngle, SpotLight1FalloffAngle);
	}

	vec3 lightSpaceScreenCoordinate_SpotLight2 = fragmentPositionLightSpace_SpotLight2.xyz / fragmentPositionLightSpace_SpotLight2.w;

	if(	0.f <= lightSpaceScreenCoordinate_SpotLight2.x && lightSpaceScreenCoordinate_SpotLight2.x <= 1.f &&
		0.f <= lightSpaceScreenCoordinate_SpotLight2.y && lightSpaceScreenCoordinate_SpotLight2.y <= 1.f)
	{
		float shadowValue_SpotLight2 = textureProj(shadowMap_SpotLight2, fragmentPositionLightSpace_SpotLight2);
		
		if(0.f < shadowValue_SpotLight2)
		{
			vec3 lightColor_SpotLight2 = CalculateSpotLightColor(SpotLight2Position, SpotLight2Direction, SpotLight2Intensity, SpotLight2CoverageAngle, SpotLight2FalloffAngle); 
			lightColor += shadowValue_SpotLight2 * lightColor_SpotLight2;
		}
	}
	else
	{
		lightColor +=  CalculateSpotLightColor(SpotLight2Position, SpotLight2Direction, SpotLight2Intensity, SpotLight2CoverageAngle, SpotLight2FalloffAngle);
	}

	vec3 lightSpaceScreenCoordinate_SpotLight3 = fragmentPositionLightSpace_SpotLight3.xyz / fragmentPositionLightSpace_SpotLight3.w;

	if(	0.f <= lightSpaceScreenCoordinate_SpotLight3.x && lightSpaceScreenCoordinate_SpotLight3.x <= 1.f &&
		0.f <= lightSpaceScreenCoordinate_SpotLight3.y && lightSpaceScreenCoordinate_SpotLight3.y <= 1.f)
	{
		float shadowValue_SpotLight3 = textureProj(shadowMap_SpotLight3, fragmentPositionLightSpace_SpotLight3);
		
		if(0.f < shadowValue_SpotLight3)
		{
			vec3 lightColor_SpotLight3 = CalculateSpotLightColor(SpotLight3Position, SpotLight3Direction, SpotLight3Intensity, SpotLight3CoverageAngle, SpotLight3FalloffAngle); 
			lightColor += shadowValue_SpotLight3 * lightColor_SpotLight3;
		}
	}
	else
	{
		lightColor +=  CalculateSpotLightColor(SpotLight3Position, SpotLight3Direction, SpotLight3Intensity, SpotLight3CoverageAngle, SpotLight3FalloffAngle);
	}
	lightColor *= diffuseReflectance;
	fragmentColor = lightColor;
}