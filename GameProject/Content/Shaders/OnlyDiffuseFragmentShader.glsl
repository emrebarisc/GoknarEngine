#version 440 core

out vec4 fragmentColor;
in vec2 textureUV;
uniform sampler2D texture0;
in vec3 fragmentPosition;
uniform vec3 viewPosition;

float fogDistance = 150.f;
float fogMinHeight = 0.f;
float fogMaxHeight = 25.f;
vec4 fogColor = vec4(0.90f, 0.90f, 0.90f, 1.f);

void main()
{
	fragmentColor = texture(texture0, textureUV);

	float depthValue = clamp(length(fragmentPosition - viewPosition) / fogDistance, 0.f, 1.f);
	if (fogMinHeight <= fragmentPosition.z) depthValue *= (fogMaxHeight - fogMinHeight) / (fragmentPosition.z - fogMinHeight);
	else depthValue = 1.f;

	fragmentColor = fragmentColor * (1.f - depthValue) + fogColor * depthValue;
}