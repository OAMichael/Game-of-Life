#version 460 core

layout(location = 0) out vec4 outColor;
layout(location = 1) in vec3 ourColor;

void main()
{
    outColor = vec4(ourColor, 1.0f);
} 