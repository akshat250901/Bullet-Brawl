#version 330

in vec2 vTexCoord; // Texture coordinates passed from the vertex shader

out vec4 FragColor; // The final color to render

// Textures
uniform sampler2D uBackgroundTexture;
uniform sampler2D uMiddlegroundTexture;
uniform sampler2D uForegroundTexture;

// Scrolling offsets
uniform float uBackgroundOffset;
uniform float uMiddlegroundOffset;
uniform float uForegroundOffset;

void main()
{
    // Calculate infinite scrolling texture coordinates for each layer
    // Using mod() function to ensure it wraps around and creates an infinite scrolling effect
    vec2 infiniteBackgroundTexCoord = mod(vTexCoord + vec2(uBackgroundOffset, 0.0), 1.0);
    vec2 infiniteMiddlegroundTexCoord = mod(vTexCoord + vec2(uMiddlegroundOffset, 0.0), 1.0);
    vec2 infiniteForegroundTexCoord = mod(vTexCoord + vec2(uForegroundOffset, 0.0), 1.0);

    // Sample colors from each texture using adjusted coordinates
    vec4 backgroundColor = texture(uBackgroundTexture, infiniteBackgroundTexCoord);
    vec4 middlegroundColor = texture(uMiddlegroundTexture, infiniteMiddlegroundTexCoord);
    vec4 foregroundColor = texture(uForegroundTexture, infiniteForegroundTexCoord);

    // Simple blending: assuming the foreground and middleground have transparency to show layers below
    FragColor = backgroundColor;
    FragColor = mix(FragColor, middlegroundColor, middlegroundColor.a);
    FragColor = mix(FragColor, foregroundColor, foregroundColor.a);
}
