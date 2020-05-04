#version 440 core

uniform vec3 viewPosition;

in vec3 fragmentPosition;

out vec4 fragmentColor;

vec4 gridColor = vec4(0.25f, 0.25f, 0.25f, 1.f);

float maxGridDistance = 250.f;

void main()
{
	if(abs(fragmentPosition.x - round(fragmentPosition.x)) < 0.005f ||
	   abs(fragmentPosition.y - round(fragmentPosition.y)) < 0.005f)
	{
		vec3 wo = viewPosition - fragmentPosition;
		fragmentColor = gridColor;
	}
	else
	{
		discard;
	}
}