// Deferred Rendering Pass Fragment Shader
#version 440 core

out vec4 fragmentColor;

in vec2 textureUV;
uniform sampler2D position_GBuffer;
uniform sampler2D normal_GBuffer;
uniform sampler2D diffuse_GBuffer;
uniform sampler2D specularAndPhong_GBuffer;

//---------------------------------------- SHADOW MAPS ----------------------------------------
uniform mat4 lightViewMatrix_DirectionalLight0;
//----------------------------------------------------------------------------------------------


//---------------------------------------- SHADOW MAPS ----------------------------------------
uniform sampler2DShadow shadowMap_DirectionalLight0;
uniform samplerCubeShadow shadowMap_PointLight0;
//----------------------------------------------------------------------------------------------

uniform vec3 PointLight0Position;
uniform vec3 PointLight0Intensity;
uniform float PointLight0Radius;
uniform bool PointLight0IsCastingShadow;
uniform vec3 DirectionalLight0Direction;
uniform vec3 DirectionalLight0Intensity;
uniform bool DirectionalLight0IsCastingShadow;

uniform vec3 viewPosition;
uniform bool isDebugging;

vec4 fragmentPositionWorldSpace;
vec4 diffuseReflectance;
vec3 specularReflectance;
vec3 vertexNormal;
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
	vec3 diffuseColor = cosThetaPrime * intensityOverDistanceSquare;

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

	vec3 diffuseColor = intensity * normalDotLightDirection;

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


void main()
{
	if(isDebugging)
	{
		if(textureUV.x < 0.5f)
		{
			if(textureUV.y < 0.5f)
			{
				fragmentColor = texture(diffuse_GBuffer, textureUV * 2.f);
			}
			else
			{
				fragmentColor = texture(position_GBuffer, (textureUV - vec2(0.f, 0.5f)) * 2.f);
			}
		}
		else
		{
			if(textureUV.y < 0.5f)
			{
				fragmentColor = texture(specularAndPhong_GBuffer, (textureUV - vec2(0.5f, 0.5f)) * 2.f);
			}
			else
			{
				fragmentColor = texture(normal_GBuffer, (textureUV - vec2(0.5f, 0.5f)) * 2.f) * 0.5f + vec4(0.5f);
			}
		}

		return;
	}

	diffuseReflectance = texture(diffuse_GBuffer, textureUV);
	
	fragmentPositionWorldSpace = vec4(texture(position_GBuffer, textureUV).xyz, 1.f);

	vec4 fragmentNormalAndPhongExponent = texture(normal_GBuffer, textureUV);
	vertexNormal = fragmentNormalAndPhongExponent.xyz;
	phongExponent = pow(2.f, fragmentNormalAndPhongExponent.a);
	
	specularReflectance = texture(specularAndPhong_GBuffer, textureUV).xyz;
//------------------------ FRAGMENT POSITION LIGHT SPACE ------------------------

	 vec4 fragmentPositionLightSpace_DirectionalLight0 = fragmentPositionWorldSpace * lightViewMatrix_DirectionalLight0; 
//-------------------------------------------------------------------------------

	 vec3 lightColor = sceneAmbient;

	if(PointLight0IsCastingShadow)
	{
		vec3 fragmentToLightVector_PointLight0 = fragmentPositionWorldSpace.xyz + vertexNormal * 0.025f - PointLight0Position;
		float shadowValue_PointLight0 = texture(shadowMap_PointLight0, vec4(fragmentToLightVector_PointLight0, 0.15f)).x;
		shadowValue_PointLight0 *= PointLight0Radius;
		if(length(fragmentToLightVector_PointLight0) < shadowValue_PointLight0)
		{
			vec3 lightColor_PointLight0 = CalculatePointLightColor(PointLight0Position, PointLight0Intensity, PointLight0Radius);
			lightColor += shadowValue_PointLight0 * lightColor_PointLight0;
		}
	}
	else
	{
			lightColor += CalculatePointLightColor(PointLight0Position, PointLight0Intensity, PointLight0Radius);
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
				lightColor += shadowValue_DirectionalLight0 * lightColor_DirectionalLight0;
			}
		}
		else
		{
			lightColor +=  CalculateDirectionalLightColor(DirectionalLight0Direction, DirectionalLight0Intensity);
		}
	}
	else
	{
		lightColor +=  CalculateDirectionalLightColor(DirectionalLight0Direction, DirectionalLight0Intensity);

	}
	fragmentColor = vec4(lightColor) * diffuseReflectance;
}