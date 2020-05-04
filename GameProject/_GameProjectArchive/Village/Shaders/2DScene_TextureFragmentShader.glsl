#version 440 core

out vec4 color;
in vec3 fragmentPosition;
in vec3 vertexNormal;
in vec2 textureUV;

uniform sampler2D texture0;

void main()
{
	vec4 diffuseReflectance = texture(texture0, textureUV);
	if (diffuseReflectance.a < 0.5f) discard;
	color = diffuseReflectance;
}