#version 450

layout(location = 0) in float fragAlpha;
layout(location = 0) out vec4 outColor;
layout(binding = 2) uniform Chroma{
    vec3 c;
} inColor;

void main()
{
    outColor = vec4(inColor.c, fragAlpha);
}