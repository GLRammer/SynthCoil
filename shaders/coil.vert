#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in float inAlpha;

layout(location = 0) out float fragAlpha;

void main()
{
    // normalize for flat rendering
    vec3 p = inPos / 12.0;
    gl_Position = vec4(p.x, p.y, p.z, 1.0);

    // Clamp alpha just to be safe
    fragAlpha = clamp(inAlpha, 0.0, 1.0);
}