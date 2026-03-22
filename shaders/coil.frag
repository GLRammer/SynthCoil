#version 450

layout(location = 0) in float fragAlpha;
layout(location = 0) out vec4 outColor;

void main()
{
    // Base color of white
    outColor = vec4(1.0, 1.0, 1.0, fragAlpha);
}