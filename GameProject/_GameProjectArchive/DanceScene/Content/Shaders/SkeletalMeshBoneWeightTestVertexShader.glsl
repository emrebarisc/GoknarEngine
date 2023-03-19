#version 440 core


layout(location = 0) in vec4 color;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in ivec4 boneIDs;
layout(location = 5) in vec4 weights;

uniform mat4 bones[100];

uniform sampler2D T_MutantNormal;

uniform mat4 worldTransformationMatrix;
uniform mat4 relativeTransformationMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float deltaTime;
uniform float elapsedTime;

uniform bool useNormalTexture = true;

flat out ivec4 outBoneIDs;
out vec4 outWeights;
out vec4 fragmentPosition;
out vec3 vertexNormal;
out vec2 textureUV;

#define MAX_BONE_SIZE_PER_VERTEX 4

void main()
{
	mat4 boneTransformationMatrix = mat4(0.f);

	for(int boneIndex = 0; boneIndex < MAX_BONE_SIZE_PER_VERTEX; ++boneIndex)
	{
		boneTransformationMatrix += bones[boneIDs[boneIndex]] * weights[boneIndex];
	}

	mat4 modelMatrix = boneTransformationMatrix * relativeTransformationMatrix * worldTransformationMatrix;
	vec4 fragmentPosition4Channel = vec4(position, 1.f) * modelMatrix;
	gl_Position = projectionMatrix * viewMatrix * fragmentPosition4Channel;
	fragmentPosition = fragmentPosition4Channel;
	textureUV = uv; 

	if(useNormalTexture)
	{
		vec3 textureNormal = normalize(texture(T_MutantNormal, textureUV).xyz * 2.f - 1.f);
		vertexNormal = normalize(textureNormal * transpose(inverse(mat3(modelMatrix))));
	}
	else
	{
		vertexNormal = normalize(normal * transpose(inverse(mat3(modelMatrix))));
	}

	outBoneIDs = boneIDs;
	outWeights = weights;
}