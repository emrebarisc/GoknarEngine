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
uniform samplerCubeShadow shadowMap_PointLight0;
//----------------------------------------------------------------------------------------------

uniform vec3 PointLight0Position;
uniform vec3 PointLight0Intensity;
uniform float PointLight0Radius;
uniform bool PointLight0IsCastingShadow;
uniform vec3 DirectionalLight0Direction;
uniform vec3 DirectionalLight0Intensity;
uniform bool DirectionalLight0IsCastingShadow;

uniform vec3 SpotLight0Position;
uniform vec3 SpotLight0Direction;
uniform vec3 SpotLight0Intensity;
uniform float SpotLight0CoverageAngle;
uniform float SpotLight0FalloffAngle;
uniform bool SpotLight0IsCastingShadow;

uniform vec3 SpotLight1Position;
uniform vec3 SpotLight1Direction;
uniform vec3 SpotLight1Intensity;
uniform float SpotLight1CoverageAngle;
uniform float SpotLight1FalloffAngle;
uniform bool SpotLight1IsCastingShadow;

uniform vec3 SpotLight2Position;
uniform vec3 SpotLight2Direction;
uniform vec3 SpotLight2Intensity;
uniform float SpotLight2CoverageAngle;
uniform float SpotLight2FalloffAngle;
uniform bool SpotLight2IsCastingShadow;

uniform vec3 SpotLight3Position;
uniform vec3 SpotLight3Direction;
uniform vec3 SpotLight3Intensity;
uniform float SpotLight3CoverageAngle;
uniform float SpotLight3FalloffAngle;
uniform bool SpotLight3IsCastingShadow;

vec3 SpotLight4Position = vec3(50.000000, 30.000000, 10.000000);
vec3 SpotLight4Direction = vec3(-0.577350, 0.577350, -0.577350);
vec3 SpotLight4Intensity = vec3(50.000000, 37.500000, 37.500000);
float SpotLight4CoverageAngle = 0.349066;
float SpotLight4FalloffAngle = 0.087266;
uniform bool SpotLight4IsCastingShadow;

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

	float normalDotLightDirection = dot(vertexNormal, wi);
	if(normalDotLightDirection < 0.f) return vec3(0.f);

	// To viewpoint vector
	vec3 wo = normalize(viewPosition - vec3(fragmentPositionWorldSpace));

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;

	vec3 intensityOverDistanceSquare = intensity / (wiLength * wiLength);

	// Diffuse
	float cosThetaPrime = max(0.f, normalDotLightDirection);
	vec3 diffuseColor = diffuseReflectance * cosThetaPrime * intensityOverDistanceSquare;

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	vec3 specularColor = specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	specularColor *= cosThetaPrime * intensityOverDistanceSquare;

	return specularColor + diffuseColor;
}


vec3 CalculateDirectionalLightColor(vec3 direction, vec3 intensity)
{
	vec3 wi = normalize(-direction);

	float normalDotLightDirection = dot(wi, vertexNormal);

	if(normalDotLightDirection < 0.f) return vec3(0.f);

	vec3 diffuseColor = intensity * max(0.f, normalDotLightDirection);

	// To viewpoint vector
	vec3 wo = normalize(viewPosition - vec3(fragmentPositionWorldSpace));

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	vec3 specularColor = specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;
	specularColor *= max(0, normalDotLightDirection) * intensity;
	return diffuseColor + specularColor;
}


vec3 CalculateSpotLightColor(vec3 position, vec3 direction, vec3 intensity, float coverageAngle, float falloffAngle)
{
	float lightMultiplier = 0.f;

	// To light vector
	vec3 wi = vec3(fragmentPositionWorldSpace) - position;
	float wiLength = length(wi);
	wi /= wiLength;

	float normalDotLightDirection = dot(wi, vertexNormal);
	if(0.f < normalDotLightDirection) return vec3(0.f);

	vec3 intensityOverDistanceSquare = intensity / (wiLength * wiLength);

	vec3 diffuseColor = max(0.f, normalDotLightDirection) * intensityOverDistanceSquare;

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
	vec3 specularColor = specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	specularColor = (1.f + specularColor) * lightMultiplier * intensityOverDistanceSquare;
	return specularColor + diffuseColor;
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

	vec3 fragmentColor = sceneAmbient;

	if(PointLight0IsCastingShadow)
	{
		vec3 fragmentToLightVector_PointLight0 = fragmentPositionWorldSpace.xyz + vertexNormal * 0.025f - PointLight0Position;
		float shadowValue_PointLight0 = texture(shadowMap_PointLight0, vec4(fragmentToLightVector_PointLight0, 0.15f)).x;
		shadowValue_PointLight0 *= PointLight0Radius;
		if(length(fragmentToLightVector_PointLight0) < shadowValue_PointLight0)
		{
			vec3 lightColor_PointLight0 = CalculatePointLightColor(PointLight0Position, PointLight0Intensity, PointLight0Radius);
			fragmentColor += shadowValue_PointLight0 * lightColor_PointLight0;
		}
	}
	else
	{
			fragmentColor += CalculatePointLightColor(PointLight0Position, PointLight0Intensity, PointLight0Radius);
	}

	if(DirectionalLight0IsCastingShadow)
	{
		vec3 lightSpaceScreenCoordinate_DirectionalLight0 = fragmentPositionLightSpace_DirectionalLight0.xyz / fragmentPositionLightSpace_DirectionalLight0.w;

		if(	0.f <= lightSpaceScreenCoordinate_DirectionalLight0.x && lightSpaceScreenCoordinate_DirectionalLight0.x <= 1.f &&
			0.f <= lightSpaceScreenCoordinate_DirectionalLight0.y && lightSpaceScreenCoordinate_DirectionalLight0.y <= 1.f)
		{
			float shadowValue_DirectionalLight0 = textureProj(shadowMap_DirectionalLight0, fragmentPositionLightSpace_DirectionalLight0);

			if(0.f < shadowValue_DirectionalLight0)
			{
				vec3 lightColor_DirectionalLight0 = CalculateDirectionalLightColor(DirectionalLight0Direction, DirectionalLight0Intensity); 
				fragmentColor += shadowValue_DirectionalLight0 * lightColor_DirectionalLight0;
			}
		}
		else
		{
			fragmentColor +=  CalculateDirectionalLightColor(DirectionalLight0Direction, DirectionalLight0Intensity);
		}
	}
	else
	{
		fragmentColor +=  CalculateDirectionalLightColor(DirectionalLight0Direction, DirectionalLight0Intensity);

	}

	if(SpotLight4IsCastingShadow)
	{
		vec3 lightSpaceScreenCoordinate_SpotLight4 = fragmentPositionLightSpace_SpotLight4.xyz / fragmentPositionLightSpace_SpotLight4.w;

		if(	0.f <= lightSpaceScreenCoordinate_SpotLight4.x && lightSpaceScreenCoordinate_SpotLight4.x <= 1.f &&
			0.f <= lightSpaceScreenCoordinate_SpotLight4.y && lightSpaceScreenCoordinate_SpotLight4.y <= 1.f)
		{
			float shadowValue_SpotLight4 = textureProj(shadowMap_SpotLight4, fragmentPositionLightSpace_SpotLight4);
			
			if(0.f < shadowValue_SpotLight4)
			{
				vec3 lightColor_SpotLight4 = CalculateSpotLightColor(SpotLight4Position, SpotLight4Direction, SpotLight4Intensity, SpotLight4CoverageAngle, SpotLight4FalloffAngle); 
				fragmentColor += shadowValue_SpotLight4 * lightColor_SpotLight4;
			}
		}
		else
		{
			fragmentColor +=  CalculateSpotLightColor(SpotLight4Position, SpotLight4Direction, SpotLight4Intensity, SpotLight4CoverageAngle, SpotLight4FalloffAngle);
		}
	}
	else
	{
		fragmentColor +=  CalculateSpotLightColor(SpotLight4Position, SpotLight4Direction, SpotLight4Intensity, SpotLight4CoverageAngle, SpotLight4FalloffAngle);
	}

	if(SpotLight0IsCastingShadow)
	{
		vec3 lightSpaceScreenCoordinate_SpotLight0 = fragmentPositionLightSpace_SpotLight0.xyz / fragmentPositionLightSpace_SpotLight0.w;

		if(	0.f <= lightSpaceScreenCoordinate_SpotLight0.x && lightSpaceScreenCoordinate_SpotLight0.x <= 1.f &&
			0.f <= lightSpaceScreenCoordinate_SpotLight0.y && lightSpaceScreenCoordinate_SpotLight0.y <= 1.f)
		{
			float shadowValue_SpotLight0 = textureProj(shadowMap_SpotLight0, fragmentPositionLightSpace_SpotLight0);
			
			if(0.f < shadowValue_SpotLight0)
			{
				vec3 lightColor_SpotLight0 = CalculateSpotLightColor(SpotLight0Position, SpotLight0Direction, SpotLight0Intensity, SpotLight0CoverageAngle, SpotLight0FalloffAngle); 
				fragmentColor += shadowValue_SpotLight0 * lightColor_SpotLight0;
			}
		}
		else
		{
			fragmentColor +=  CalculateSpotLightColor(SpotLight0Position, SpotLight0Direction, SpotLight0Intensity, SpotLight0CoverageAngle, SpotLight0FalloffAngle);
		}
	}
	else
	{
		fragmentColor +=  CalculateSpotLightColor(SpotLight0Position, SpotLight0Direction, SpotLight0Intensity, SpotLight0CoverageAngle, SpotLight0FalloffAngle);
	}

	if(SpotLight1IsCastingShadow)
	{
		vec3 lightSpaceScreenCoordinate_SpotLight1 = fragmentPositionLightSpace_SpotLight1.xyz / fragmentPositionLightSpace_SpotLight1.w;

		if(	0.f <= lightSpaceScreenCoordinate_SpotLight1.x && lightSpaceScreenCoordinate_SpotLight1.x <= 1.f &&
			0.f <= lightSpaceScreenCoordinate_SpotLight1.y && lightSpaceScreenCoordinate_SpotLight1.y <= 1.f)
		{
			float shadowValue_SpotLight1 = textureProj(shadowMap_SpotLight1, fragmentPositionLightSpace_SpotLight1);
			
			if(0.f < shadowValue_SpotLight1)
			{
				vec3 lightColor_SpotLight1 = CalculateSpotLightColor(SpotLight1Position, SpotLight1Direction, SpotLight1Intensity, SpotLight1CoverageAngle, SpotLight1FalloffAngle); 
				fragmentColor += shadowValue_SpotLight1 * lightColor_SpotLight1;
			}
		}
		else
		{
			fragmentColor +=  CalculateSpotLightColor(SpotLight1Position, SpotLight1Direction, SpotLight1Intensity, SpotLight1CoverageAngle, SpotLight1FalloffAngle);
		}
	}
	else
	{
		fragmentColor +=  CalculateSpotLightColor(SpotLight1Position, SpotLight1Direction, SpotLight1Intensity, SpotLight1CoverageAngle, SpotLight1FalloffAngle);
	}

	if(SpotLight2IsCastingShadow)
	{
		vec3 lightSpaceScreenCoordinate_SpotLight2 = fragmentPositionLightSpace_SpotLight2.xyz / fragmentPositionLightSpace_SpotLight2.w;

		if(	0.f <= lightSpaceScreenCoordinate_SpotLight2.x && lightSpaceScreenCoordinate_SpotLight2.x <= 1.f &&
			0.f <= lightSpaceScreenCoordinate_SpotLight2.y && lightSpaceScreenCoordinate_SpotLight2.y <= 1.f)
		{
			float shadowValue_SpotLight2 = textureProj(shadowMap_SpotLight2, fragmentPositionLightSpace_SpotLight2);
			
			if(0.f < shadowValue_SpotLight2)
			{
				vec3 lightColor_SpotLight2 = CalculateSpotLightColor(SpotLight2Position, SpotLight2Direction, SpotLight2Intensity, SpotLight2CoverageAngle, SpotLight2FalloffAngle); 
				fragmentColor += shadowValue_SpotLight2 * lightColor_SpotLight2;
			}
		}
		else
		{
			fragmentColor +=  CalculateSpotLightColor(SpotLight2Position, SpotLight2Direction, SpotLight2Intensity, SpotLight2CoverageAngle, SpotLight2FalloffAngle);
		}
	}
	else
	{
		fragmentColor +=  CalculateSpotLightColor(SpotLight2Position, SpotLight2Direction, SpotLight2Intensity, SpotLight2CoverageAngle, SpotLight2FalloffAngle);
	}

	if(SpotLight3IsCastingShadow)
	{
		vec3 lightSpaceScreenCoordinate_SpotLight3 = fragmentPositionLightSpace_SpotLight3.xyz / fragmentPositionLightSpace_SpotLight3.w;

		if(	0.f <= lightSpaceScreenCoordinate_SpotLight3.x && lightSpaceScreenCoordinate_SpotLight3.x <= 1.f &&
			0.f <= lightSpaceScreenCoordinate_SpotLight3.y && lightSpaceScreenCoordinate_SpotLight3.y <= 1.f)
		{
			float shadowValue_SpotLight3 = textureProj(shadowMap_SpotLight3, fragmentPositionLightSpace_SpotLight3);
			
			if(0.f < shadowValue_SpotLight3)
			{
				vec3 lightColor_SpotLight3 = CalculateSpotLightColor(SpotLight3Position, SpotLight3Direction, SpotLight3Intensity, SpotLight3CoverageAngle, SpotLight3FalloffAngle); 
				fragmentColor += shadowValue_SpotLight3 * lightColor_SpotLight3;
			}
		}
		else
		{
			fragmentColor +=  CalculateSpotLightColor(SpotLight3Position, SpotLight3Direction, SpotLight3Intensity, SpotLight3CoverageAngle, SpotLight3FalloffAngle);
		}
	}
	else
	{
		fragmentColor +=  CalculateSpotLightColor(SpotLight3Position, SpotLight3Direction, SpotLight3Intensity, SpotLight3CoverageAngle, SpotLight3FalloffAngle);
	}
	fragmentColor *= diffuseReflectance;
}