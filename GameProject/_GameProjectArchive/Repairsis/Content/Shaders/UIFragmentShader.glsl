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
uniform sampler2D UITexture;

vec4 diffuseReflectance;

void main()
{
	diffuseReflectance = texture(UITexture, textureUV);
	if (diffuseReflectance.a < 0.5f) discard;
	
	fragmentColor = diffuseReflectance;
}