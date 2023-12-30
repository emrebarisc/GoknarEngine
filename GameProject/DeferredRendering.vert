
#version 440 core

layout(location = 0) in vec4 color;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

out vec2 textureUV;

void main()
{
	gl_Position = vec4(position, 1.f);
	textureUV = 0.5f * gl_Position.xy + vec2(0.5f);
}
