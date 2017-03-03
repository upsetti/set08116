#version 440

// The model matrix
uniform mat4 M;
// The transformation matrix
uniform mat4 MVP;
// The normal matrix
uniform mat3 N;

// Incoming position
layout(location = 0) in vec3 position_in;
// Incoming normal
layout(location = 2) in vec3 normal_in;
// Incoming texture coordinate
layout(location = 10) in vec2 tex_coord_in;

// Outgoing vertex position
layout(location = 0) out vec3 position;
// Outgoing transformed normal
layout(location = 1) out vec3 normal;
// Outgoing texture coordinate
layout(location = 2) out vec2 tex_coord;

void main() {
  // Set position
  gl_Position = MVP * vec4(position_in, 1);
  // *********************************
  // Output other values to fragment shader
  position = vec3(M * vec4(position_in, 1.0));
  normal = N * normal_in;
  tex_coord = tex_coord_in;
  // *********************************
}