// DefaultFragmentShader

#version 440 core


// Base Material Variables
uniform vec3 ambientReflectance;
uniform vec3 specularReflectance;
uniform float phongExponent;

uniform float translucency;

vec3 sceneAmbient = vec3(0.000000, 0.000000, 0.000000);

void main()
{
	vec3 lightIntensity = sceneAmbient * ambientReflectance;
	fragmentColor = vec4(lightIntensity, 1.f) * diffuseReflectance;
}