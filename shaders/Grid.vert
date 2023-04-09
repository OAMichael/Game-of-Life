#version 460 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 aColor;

layout(location = 1) out vec3 ourColor;

uniform mat4 transform;

void main()
{
    ourColor = aColor;
    gl_Position = transform * vec4(inPos, 0.0f, 1.0f);
}