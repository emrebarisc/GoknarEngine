#version 440 core


out vec3 color;
in vec3 fragmentPosition;
in vec3 vertexNormal;
uniform vec3 viewPosition;

// Base Material Variables
uniform vec3 ambientReflectance;
uniform vec3 diffuseReflectance;
uniform vec3 specularReflectance;
uniform float phongExponent;

vec3 sceneAmbient = vec3(0.392157, 0.392157, 0.392157);

vec3 CalculatePointLightColor(vec3 position, vec3 intensity)
{
	// To light vector
	vec3 wi = position - fragmentPosition;
	float wiLength = length(wi);
	wi /= wiLength;

	// To viewpoint vector
	vec3 wo = viewPosition - fragmentPosition;
	float woLength = length(wo);
	wo /= woLength;

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;

	float inverseDistanceSquare = 1 / (wiLength * wiLength);

	// Diffuse
	float cosThetaPrime = max(0.f, dot(wi, vertexNormal));
	vec3 color = diffuseReflectance * cosThetaPrime;

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	color += specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	color *= intensity * inverseDistanceSquare;

	return clamp(color, 0.f, 1.f);
}

vec3 PointLight0Position = vec3(-7.500000, 7.500000, 7.500000);
vec3 PointLight0Intensity = vec3(100.000000, 100.000000, 100.000000);
vec3 PointLight1Position = vec3(7.500000, -7.500000, 10.000000);
vec3 PointLight1Intensity = vec3(100.000000, 100.000000, 100.000000);

void main()
{
	vec3 lightColor = sceneAmbient * ambientReflectance;
	lightColor += CalculatePointLightColor(PointLight0Position, PointLight0Intensity);
	lightColor += CalculatePointLightColor(PointLight1Position, PointLight1Intensity);
	color = lightColor;

}