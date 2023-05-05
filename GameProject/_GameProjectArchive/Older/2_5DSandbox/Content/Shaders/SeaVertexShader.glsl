// 2DVertexShader.glsl

#version 440 core

layout(location = 0) in vec4 color;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

// Transformation matrix is calculated by multiplying  
// world and relative transformation matrices
uniform mat4 worldTransformationMatrix;
uniform mat4 relativeTransformationMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 viewPosition;

uniform float deltaTime;
uniform float elapsedTime;

out vec3 fragmentPosition;
out vec3 vertexNormal;
out vec2 textureUV;

float Noise(vec2 p, int seed)
{
	return fract(sin(dot(p, vec2(seed * 0.12765f, seed * 0.76583f))) * seed * 234.12312f);
}

void main()
{
	mat4 transformationMatrix = relativeTransformationMatrix * worldTransformationMatrix;
	vec4 fragmentPosition4Channel = vec4(position, 1.f) * transformationMatrix;
	vec4 transformedPosition = projectionMatrix * viewMatrix * fragmentPosition4Channel;
	gl_Position = transformedPosition + vec4(0.f, 0.f, transformedPosition.x, 0.f);
	vertexNormal = vec3(vec4(normal, 0.f) * transpose(inverse(transformationMatrix)));
	fragmentPosition = vec3(fragmentPosition4Channel);

	textureUV = vec2(uv.x, uv.y);
}