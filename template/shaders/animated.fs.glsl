

#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// Output color
layout(location = 0) out vec4 color;

void main()
{
    float electricityIntensity = 1000.f;
    float electricityFrequency = 60.0;
    float electricityOffset = sin(gl_FragCoord.x * electricityFrequency);
    // Set electricity color to grayscale
    vec3 electricityColor = vec3(0.0, 0.0, 0.001);
    
    vec4 textureColor = texture(sampler0, vec2(texcoord.x, texcoord.y));

    // Add electricity effect to the final color
    color = vec4(fcolor, 1.0) * textureColor;

    color.xyz += electricityIntensity * electricityOffset * electricityColor;
}

