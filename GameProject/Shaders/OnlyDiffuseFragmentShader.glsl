#version 440 core

out vec4 fragmentColor;
in vec2 textureUV;
uniform sampler2D texture0;
in vec3 fragmentPosition;
uniform vec3 viewPosition;

float fogDistance = 75.f;

void main()
{
	vec4 textureColor = texture(texture0, textureUV);
	vec3 depthValue = vec3(clamp(length(fragmentPosition - viewPosition) / fogDistance, 0.f, 1.f));

	fragmentColor = vec4(textureColor.rgb * (1 - depthValue) + vec3(1.f) * depthValue, 1.f);
}