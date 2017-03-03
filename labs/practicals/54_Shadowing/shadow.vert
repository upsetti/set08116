#version 440

// The transformation matrix
uniform mat4 MVP;
// The model transformation
uniform mat4 M;
// The normal matrix
uniform mat3 N;
// The light transformation matrix
uniform mat4 lightMVP;

// Incoming position
layout (location = 0) in vec3 position_in;
// Incoming normal
layout (location = 2) in vec3 normal_in;
// Incoming texture coordinate
layout (location = 10) in vec2 tex_coord_in;

// Outgoing vertex position
layout (location = 0) out vec3 position;
// Outgoing normal
layout (location = 1) out vec3 normal;
// Outgoing texture coordinate
layout (location = 2) out vec2 tex_coord;
// Outgoing position in light space
layout (location = 3) out vec4 light_space_pos;

void main()
{
    // Transform position into screen space
    gl_Position = MVP * vec4(position_in, 1.0);
    // Transform position into world space
    position = (M * vec4(position_in, 1.0)).xyz;
    // Transform normal
    normal = N * normal_in;
    // Pass through texture coordinate
    tex_coord = tex_coord_in;
    // *********************************
    // Transform position into light space
		light_space_pos = lightMVP * vec4(position_in, 1.0);
    // *********************************
}