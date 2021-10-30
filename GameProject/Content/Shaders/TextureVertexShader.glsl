#version 440 core


layout(location = 0) in vec4 color;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
uniform mat4 worldTransformationMatrix;
uniform mat4 relativeTransformationMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float deltaTime;
uniform float elapsedTime;

out vec2 textureUV;
out vec3 fragmentPosition;
out vec3 vertexNormal;

void main()
{
	vec4 fragmentPosition4Channel = vec4(position, 1.f) * worldTransformationMatrix * relativeTransformationMatrix;
	gl_Position = projectionMatrix * viewMatrix * fragmentPosition4Channel;
	fragmentPosition = vec3(fragmentPosition4Channel);

	vertexNormal = vec3(vec4(normal, 0.f) * transpose(inverse(worldTransformationMatrix * relativeTransformationMatrix)));

	textureUV = vec2(uv.x, 1 - uv.y);
}