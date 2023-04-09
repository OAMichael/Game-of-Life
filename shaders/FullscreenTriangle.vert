#version 460 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUv;

layout(location = 1) out vec2 outUv;

uniform mat4 transform;

void main()
{   
    outUv = inUv;
    gl_Position = transform * vec4(inPos, 0.0f, 1.0f);
}