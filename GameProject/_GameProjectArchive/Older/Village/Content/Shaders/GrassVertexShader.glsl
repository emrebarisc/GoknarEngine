// GrassVertexShader.glsl

#version 440 core


layout(location = 0) in vec4 color;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

uniform mat4 transformationMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float deltaTime;
uniform float elapsedTime;

out vec3 fragmentPosition;
out vec3 vertexNormal;
out vec2 textureUV;

vec3 windDirection = vec3(0.70710678118f, 0.70710678118f, 0.f);
float windStrength = 0.25f;
float windSpeed = 0.6f;

void main()
{
	vec4 fragmentPosition4Channel = vec4(position, 1.f) * transformationMatrix;
	vec4 calculatedPosition = projectionMatrix * viewMatrix * fragmentPosition4Channel;
	fragmentPosition = vec3(fragmentPosition4Channel);

	vec3 windDisposition = windDirection * windStrength * sin(elapsedTime * windSpeed) * vec3(color);

	gl_Position = calculatedPosition + vec4(windDisposition, 0.f);

	vertexNormal = vec3(vec4(normal, 0.f) * transpose(inverse(transformationMatrix)));
	textureUV = vec2(uv.x, 1 - uv.y);
}
