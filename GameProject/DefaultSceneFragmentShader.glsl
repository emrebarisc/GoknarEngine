// DefaultFragmentShader

#version 440 core


// Base Material Variables
uniform vec3 ambientReflectance;
uniform vec3 specularReflectance;
uniform float phongExponent;

vec3 sceneAmbient = vec3(0.039216, 0.039216, 0.039216);

void main()
{
	fragmentColor = sceneAmbient * ambientReflectance * diffuseReflectance;

}