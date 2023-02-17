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

out vec4 fragmentPosition;
out vec3 vertexNormal;
out vec2 textureUV;

void main()
{

	mat4 modelMatrix = relativeTransformationMatrix * worldTransformationMatrix;
	vec4 fragmentPosition4Channel = vec4(position, 1.f) * modelMatrix;
	gl_Position = projectionMatrix * viewMatrix * fragmentPosition4Channel;
	fragmentPosition = fragmentPosition4Channel;
	textureUV = uv; 

	vertexNormal = normalize(normal * transpose(inverse(mat3(modelMatrix))));

}
