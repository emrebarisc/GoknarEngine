#version 440 core


out vec3 fragmentColor;
in vec3 fragmentPosition;
in vec3 vertexNormal;
uniform vec3 viewPosition;
// Base Material Variables
uniform vec3 ambientReflectance;
uniform vec3 diffuseReflectance;
uniform vec3 specularReflectance;
uniform float phongExponent;

uniform float deltaTime;
uniform float elapsedTime;

vec3 sceneAmbient = vec3(0.392157, 0.392157, 0.392157);

float Noise(vec2 p, int seed)
{
	return fract(sin(dot(p, vec2(seed * 0.12765f, seed * 0.76583f))) * seed * 234.12312f);
}

vec3 CalculateDirectionalLightColor(vec3 direction, vec3 intensity)
{
	vec3 wi = -direction;
	float wiLength = length(wi);
	wi /= wiLength;

	if (dot(vertexNormal, wi) < 0.f) return vec3(0.f, 0.f, 0.f);

	float normalDotLightDirection = dot(vertexNormal, wi);

	vec3 color = diffuseReflectance * max(0, normalDotLightDirection);

	// To viewpoint vector
	vec3 wo = viewPosition - fragmentPosition;
	float woLength = length(wo);
	wo /= woLength;

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;
	float inverseDistanceSquare = 1 / (wiLength * wiLength);

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	color += specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	color *= intensity * inverseDistanceSquare;

	return clamp(color, 0.f, 1.f);
}

vec3 DirectionalLight0Direction = vec3(0.577350, 0.577350, -0.577350);
vec3 DirectionalLight0Intensity = vec3(0.750000, 0.742500, 0.622500);

void main()
{
	vec3 lightColor = sceneAmbient * ambientReflectance;
	lightColor += CalculateDirectionalLightColor(DirectionalLight0Direction, DirectionalLight0Intensity);
	fragmentColor = lightColor;// vec3(Noise(floor(fragmentPosition.xy), 1612) * sin(elapsedTime * 0.0001f + Noise(floor(fragmentPosition.xy), 645) * 3.1415f));
}