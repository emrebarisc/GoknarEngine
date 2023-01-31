#version 440 core


layout(location = 0) in vec4 color;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in int boneIDs[4];
layout(location = 5) in float weights[4];

uniform mat4 bones[100];

uniform mat4 worldTransformationMatrix;
uniform mat4 relativeTransformationMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float deltaTime;
uniform float elapsedTime;

flat out int outBoneIDs[4];
out float outWeights[4];
out vec4 fragmentPosition;
out vec3 vertexNormal;
out vec2 textureUV;

#define MAX_BONE_SIZE_PER_VERTEX 4

void main()
{
	mat4 boneTransformationMatrix = mat4(1.f);

	for(int boneIndex = 0; boneIndex < MAX_BONE_SIZE_PER_VERTEX; ++boneIndex)
	{
		boneTransformationMatrix += bones[boneIDs[0]] * weights[0];
		boneTransformationMatrix += bones[boneIDs[1]] * weights[1];
		boneTransformationMatrix += bones[boneIDs[2]] * weights[2];
		boneTransformationMatrix += bones[boneIDs[3]] * weights[3];
	}

	mat4 modelMatrix = boneTransformationMatrix * relativeTransformationMatrix * worldTransformationMatrix;

	vec4 fragmentPosition4Channel = vec4(position, 1.f) * modelMatrix;
	gl_Position = projectionMatrix * viewMatrix * fragmentPosition4Channel;
	fragmentPosition = fragmentPosition4Channel;
	textureUV = uv;

	vertexNormal = normalize(normal * transpose(inverse(mat3(modelMatrix))));

	outBoneIDs = boneIDs;
	outWeights = weights;
}
