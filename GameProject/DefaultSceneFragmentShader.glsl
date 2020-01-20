#version 440 core


out vec3 color;
in vec3 fragmentPosition;
in vec3 vertexNormal;
in vec3 vertexColor;
uniform vec3 viewPosition;
// Base Material Variables
uniform vec3 ambientReflectance;
uniform vec3 diffuseReflectance;
uniform vec3 specularReflectance;
uniform float phongExponent;

vec3 sceneAmbient = vec3(0.000000, 0.000000, 0.000000);

void main()
{
	vec3 lightColor = sceneAmbient * ambientReflectance;
	color = lightColor;

}