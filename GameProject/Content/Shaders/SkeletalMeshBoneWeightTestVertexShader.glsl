#version 440 core


layout(location = 0) in vec4 color;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in ivec4 boneIDs;
layout(location = 5) in vec4 weights;

uniform mat4 worldTransformationMatrix;
uniform mat4 relativeTransformationMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float deltaTime;
uniform float elapsedTime;

out vec4 fragmentPosition;
out vec3 vertexNormal;
out vec2 textureUV;

flat out ivec4 outBoneIDs;
out vec4 outWeights;

void main()
{
	vec4 fragmentPosition4Channel = vec4(position, 1.f) * relativeTransformationMatrix * worldTransformationMatrix;
	gl_Position = projectionMatrix * viewMatrix * fragmentPosition4Channel;
	fragmentPosition = fragmentPosition4Channel;
	textureUV = vec2(uv.x, uv.y);

	vertexNormal = vec3(vec4(normal, 0.f) * transpose(inverse(relativeTransformationMatrix * worldTransformationMatrix)));

	outBoneIDs = boneIDs;
	outWeights = weights;
}
