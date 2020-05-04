#version 440 core

out vec4 fragmentColor;
in vec3 fragmentPosition;
in vec3 vertexNormal;
in vec2 textureUV;
uniform vec3 viewPosition;

// Base Material Variables
uniform vec3 ambientReflectance;
uniform vec3 specularReflectance;
uniform float phongExponent;
uniform sampler2D texture0;

vec4 diffuseReflectance;

vec3 sceneAmbient = vec3(0.2, 0.2, 0.2);

vec3 CalculateDirectionalLightColor(vec3 direction, vec3 intensity)
{
	vec3 wi = -direction;
	float wiLength = length(wi);
	wi /= wiLength;

	if(dot(vertexNormal, wi) < 0.f) return vec3(0.f, 0.f, 0.f);

	float normalDotLightDirection = dot(vertexNormal, wi);

	vec3 color = vec3(diffuseReflectance) * max(0, normalDotLightDirection);

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

vec3 DirectionalLight0Direction = vec3(0.57735f, 0.57735f, -0.57735f);
vec3 DirectionalLight0Intensity = vec3(1.0, 0.99, 0.83);

float fogDistance = 150.f;
float fogMinHeight = 0.f;
float fogMaxHeight = 25.f;
vec3 fogColor = vec3(0.90f);

void main()
{
	diffuseReflectance = texture(texture0, textureUV);
	if (diffuseReflectance.a < 0.5f) discard;

	vec3 lightColor = sceneAmbient * ambientReflectance;
	lightColor += CalculateDirectionalLightColor(DirectionalLight0Direction, DirectionalLight0Intensity);

	float depthValue = clamp(length(fragmentPosition - viewPosition) / fogDistance, 0.f, 1.f);
	depthValue = (fogMaxHeight - fogMinHeight) / (fragmentPosition.z - fogMinHeight) * depthValue;

	fragmentColor = vec4(lightColor * (1.f - depthValue) + fogColor * depthValue, diffuseReflectance.a);
}