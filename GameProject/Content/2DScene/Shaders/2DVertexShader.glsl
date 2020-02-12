#version 440 core

layout(location = 0) in vec4 color;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

// Transformation matrix is calculated by multiplying  
// world and relative transformation matrices
uniform mat4 transformationMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 fragmentPosition;
out vec3 vertexNormal;
out vec2 textureUV;

void main()
{
	vec4 fragmentPosition4Channel = vec4(position, 1.f) * transformationMatrix;
	gl_Position = projectionMatrix * viewMatrix * fragmentPosition4Channel;
	vertexNormal = vec3(vec4(normal, 0.f) * transpose(inverse(transformationMatrix)));
	fragmentPosition = vec3(fragmentPosition4Channel);

	textureUV = vec2(uv.x, uv.y);
}