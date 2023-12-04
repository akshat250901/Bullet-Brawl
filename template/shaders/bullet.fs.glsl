#version 330


// From Vertex Shader
in vec3 vcolor;

// Application data
uniform vec3 color;
uniform int light_up_red;
uniform int light_up_green;

// Output color
layout(location = 0) out vec4 out_color;

void main()
{
 
    out_color = vec4(color * vcolor, 1.0);

    if (light_up_green == 1)
    {
        out_color.xyz = 20.f * vec3(0.0, 1.0, 0.0);
    } else if (light_up_red == 1)
    {
        out_color.xyz = 20.f * vec3(1.0, 0.0, 0.0);
    }
}


