// DefaultFragmentShader

#version 440 core

out vec3 fragmentColor;
in vec4 fragmentPosition;
in vec3 vertexNormal;
in vec2 textureUV;
uniform vec3 viewPosition;
uniform vec3 diffuseReflectance;
// Base Material Variables
uniform vec3 ambientReflectance;
uniform vec3 specularReflectance;
uniform float phongExponent;
uniform float deltaTime;
uniform float elapsedTime;

flat in ivec4 outBoneIDs;
in vec4 outWeights;

void main()
{
	int boneIndex = int(elapsedTime) % 4;

	fragmentColor = vec3(0.f);
    for (int i = 0 ; i < 4 ; i++)
    {
		if (outBoneIDs[i] == boneIndex)
		{
			if (outWeights[i] > 0.7)
			{
			   fragmentColor = vec3(1.0, 0.0, 0.0) * outWeights[i];
			}
			else if (outWeights[i] >= 0.4 && outWeights[i] <= 0.7)
			{
			   fragmentColor = vec3(0.0, 1.0, 0.0) * outWeights[i];
			}
			else if (outWeights[i] >= 0.1)
			{
			   fragmentColor = vec3(1.0, 1.0, 0.0) * outWeights[i];
			}

			break;
		}
	}
}