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
vec3 sceneAmbient = vec3(0.392157, 0.392157, 0.392157);

flat in int outBoneIDs[4];
in float outWeights[4];

vec3 CalculateDirectionalLightColor(vec3 direction, vec3 intensity)
{
	vec3 wi = -direction;
	float wiLength = length(wi);
	wi /= wiLength;

	if(dot(vertexNormal, wi) < 0.f) return vec3(0.f, 0.f, 0.f);

	float normalDotLightDirection = dot(vertexNormal, wi);

	vec3 color = vec3(0.f);

	// To viewpoint vector
	vec3 wo = viewPosition - vec3(fragmentPosition);
	float woLength = length(wo);
	wo /= woLength;

	// Half vector
	vec3 halfVector = (wi + wo) * 0.5f;
	float inverseDistanceSquare = 1 / (wiLength * wiLength);

	// Specular
	float cosAlphaPrimeToThePowerOfPhongExponent = pow(max(0.f, dot(vertexNormal, halfVector)), phongExponent);
	color += specularReflectance * cosAlphaPrimeToThePowerOfPhongExponent;

	color *= diffuseReflectance  * max(0, normalDotLightDirection) * intensity * inverseDistanceSquare;

	return clamp(color, 0.f, 1.f);
}

vec3 DirectionalLight0Direction = vec3(-0.577350, 0.577350, -0.577350);
vec3 DirectionalLight0Intensity = vec3(1.500000, 1.485000, 1.245000);

void main()
{
	vec4 textureColor = texture(T_MutantDiffuse, textureUV); 
	diffuseReflectance = vec3(textureColor); 

	vec3 lightColor = sceneAmbient * ambientReflectance * diffuseReflectance;
	lightColor += CalculateDirectionalLightColor(DirectionalLight0Direction, DirectionalLight0Intensity);
	fragmentColor = lightColor;
}