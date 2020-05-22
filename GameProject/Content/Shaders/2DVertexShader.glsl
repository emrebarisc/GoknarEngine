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

out vec3 fragmentPosition;
out vec3 vertexNormal;
out vec2 textureUV;

void main()
{
	vec3 posToView = position - viewPosition;
	posToView.z = 0.f;

	float radian = atan(posToView.y, posToView.x);

	float cosAngle = cos(radian);
	float sinAngle = sin(radian);

	mat4 rotation;
	rotation[0][0] = cosAngle;
	rotation[0][1] = sinAngle;
	rotation[0][2] = 0.f;
	rotation[0][3] = 0.f;

	rotation[1][0] = -sinAngle;
	rotation[1][1] = cosAngle;
	rotation[1][2] = 0.f;
	rotation[1][3] = 0.f;

	rotation[2][0] = 0.f;
	rotation[2][1] = 0.f;
	rotation[2][2] = 1.f;
	rotation[2][3] = 0.f;

	rotation[3][0] = 0.f;
	rotation[3][1] = 0.f;
	rotation[3][2] = 0.f;
	rotation[3][3] = 1.f;

	mat4 transformationMatrix = rotation * relativeTransformationMatrix * worldTransformationMatrix;
	vec4 fragmentPosition4Channel = vec4(position, 1.f) * transformationMatrix;
	gl_Position = projectionMatrix * viewMatrix * fragmentPosition4Channel;
	vertexNormal = vec3(vec4(normal, 0.f) * transpose(inverse(transformationMatrix)));
	fragmentPosition = vec3(fragmentPosition4Channel);

	textureUV = vec2(uv.x, uv.y);
}