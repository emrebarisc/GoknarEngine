#version 440 core

uniform vec3 ambientReflectance;
uniform vec3 specularReflectance;
uniform float phongExponent;

out vec3 fragmentColor;
in vec4 fragmentPosition;
in vec3 vertexNormal;
in vec2 textureUV;
uniform vec3 viewPosition;
uniform sampler2D T_MutantDiffuse;
vec3 diffuseReflectance;

uniform int currentBoneIndex = 0;

flat in ivec4 outBoneIDs;
in vec4 outWeights;

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
vec3 sceneAmbient = vec3(0.392157, 0.392157, 0.392157);

vec3 CalculateSpotLightColor(vec3 position, vec3 direction, vec3 intensity, float coverageAngle, float falloffAngle)
{
	float lightMultiplier = 0.f;
	vec3 specular = vec3(0.f, 0.f, 0.f);

	// To light vector
	vec3 wi = vec3(fragmentPosition) - position;
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
	vec3 wo = normalize(viewPosition - vec3(fragmentPosition));

	// Half vector
	vec3 halfVector = (-wi + wo) * 0.5f;

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	specular = specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	vec3 color = (1.f + specular) * lightMultiplier * intensity * inverseDistanceSquare;
	return clamp(color, 0.f, 1.f);
}

vec3 SpotLight4Position = vec3(20.000000, -20.000000, 20.000000);
vec3 SpotLight4Direction = vec3(-0.577350, 0.577350, -0.577350);
vec3 SpotLight4Intensity = vec3(500.000000, 375.000000, 375.000000);
float SpotLight4CoverageAngle = 0.349066;
float SpotLight4FalloffAngle = 0.174533;

void main()
{
	vec4 textureColor = texture(T_MutantDiffuse, textureUV); 
	diffuseReflectance = vec3(textureColor);

	vec3 lightColor = sceneAmbient * ambientReflectance;
	lightColor += CalculateSpotLightColor(SpotLight0Position, SpotLight0Direction, SpotLight0Intensity, SpotLight0CoverageAngle, SpotLight0FalloffAngle);
	lightColor += CalculateSpotLightColor(SpotLight1Position, SpotLight1Direction, SpotLight1Intensity, SpotLight1CoverageAngle, SpotLight1FalloffAngle);
	lightColor += CalculateSpotLightColor(SpotLight2Position, SpotLight2Direction, SpotLight2Intensity, SpotLight2CoverageAngle, SpotLight2FalloffAngle);
	lightColor += CalculateSpotLightColor(SpotLight3Position, SpotLight3Direction, SpotLight3Intensity, SpotLight3CoverageAngle, SpotLight3FalloffAngle);
	lightColor += CalculateSpotLightColor(SpotLight4Position, SpotLight4Direction, SpotLight4Intensity, SpotLight4CoverageAngle, SpotLight4FalloffAngle);
	lightColor *= diffuseReflectance;

	fragmentColor = lightColor;

	bool debugBones = false;
	if(debugBones)
	{
		vec3 boneWeight = vec3(0.f, 0.f, 1.f);
		for(int boneIndex = 0; boneIndex < 4; ++boneIndex)
		{
			if(outBoneIDs[boneIndex] == currentBoneIndex && 0.f < outWeights[boneIndex])
			{
				float weight = outWeights[boneIndex];

				if(weight < 0.25f)
				{
					boneWeight = vec3(1.f, 1.f, 0.f) * weight;
				}
				else if(weight < 0.5f)
				{
					boneWeight = vec3(0.f, 1.f, 0.f) * weight;
				}
				else
				{
					boneWeight = vec3(1.f, 0.f, 0.f) * weight;
				}
				break;
			}
		}

		fragmentColor = boneWeight * 0.9f + diffuseReflectance * 0.1f;
	}
}