#version 440 core


out vec4 fragmentColor;
in vec3 fragmentPosition;
in vec3 vertexNormal;
in vec2 textureUV;
uniform vec3 viewPosition;

// Base Material Variables
uniform vec3 ambientReflectance;
uniform vec3 diffuseReflectance;
uniform vec3 specularReflectance;
uniform float phongExponent;
uniform sampler2D DirectionalLight0ShadowMap;
uniform sampler2D sampleTexture;

vec3 diffuseColor;
vec3 sceneAmbient = vec3(0.392157, 0.392157, 0.392157);

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

vec3 DirectionalLight0Direction = vec3(-0.577350, 0.577350, -0.577350);
vec3 DirectionalLight0Intensity = vec3(0.750000, 0.742500, 0.622500);

void main()
{
	diffuseColor = ((texture(DirectionalLight0ShadowMap, textureUV)).rgb);// +(texture(DirectionalLight0ShadowMap, textureUV)).rgb) * 0.5f;
	fragmentColor = vec4(diffuseColor, 1.f);

	//vec3 lightColor = sceneAmbient * ambientReflectance;
	//lightColor += CalculateDirectionalLightColor(DirectionalLight0Direction, DirectionalLight0Intensity);

	//fragmentColor = vec4(lightColor, 1.f);
}