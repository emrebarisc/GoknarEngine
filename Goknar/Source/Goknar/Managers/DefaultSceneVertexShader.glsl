#version 440 core
uniform mat4 modelMatrix;
uniform mat4 viewProjectionMatrix;
uniform float deltaTime;
uniform float elapsedTime;

out mat4 finalModelMatrix;
out vec4 fragmentPositionWorldSpace;
out vec4 fragmentPositionScreenSpace;
out vec3 vertexNormal;
out vec2 textureUV;

void main()
{
	finalModelMatrix = modelMatrix;
	fragmentPositionWorldSpace = vec4(position, 1.f) * finalModelMatrix;
	fragmentPositionScreenSpace = fragmentPositionWorldSpace * viewProjectionMatrix;
	
	gl_Position = fragmentPositionScreenSpace;
	textureUV = vec2(uv.x, 1.f - uv.y); 

	vertexNormal = normalize(normal * transpose(inverse(mat3(finalModelMatrix))));

}
