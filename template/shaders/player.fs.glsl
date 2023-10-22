#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform vec3 glowColor; // Color of the glow.
uniform float glowIntensity; // How much glow to apply. 0 for none, higher values for more glow.

// Output color
layout(location = 0) out vec4 color;

void main()
{
    vec4 textureColor = texture(sampler0, vec2(texcoord.x, texcoord.y));
    vec3 glowEffect = glowColor * glowIntensity;

    color = vec4(fcolor + glowEffect, 1.0) * textureColor;
}