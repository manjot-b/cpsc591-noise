#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;
uniform vec3 lightPos;

out vec3 modelPos;
out vec3 normal;
out vec3 toLight;

void main()
{
	vec4 worldPos = model * vec4(inPosition, 1.0);
    gl_Position = perspective * view * worldPos;
	modelPos = inPosition;
	normal = normalize((model * vec4(inNormal, 0)).xyz);
	toLight = normalize(lightPos - worldPos.xyz);
}
