#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform float animation_frame;
uniform float animation_type;
uniform float sprite_width;
uniform float sprite_height;

void main()
{
	texcoord = in_texcoord;
	texcoord.y += sprite_height * animation_type;
	texcoord.x += sprite_width * animation_frame;
	float result_x = in_position.x;

	vec3 pos = projection * transform * vec3(result_x, in_position.y, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}