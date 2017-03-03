#version 440

// The model matrix
uniform mat4 M;
// The transformation matrix
uniform mat4 MVP;
// The normal matrix
uniform mat3 N;
// Material colour
uniform vec4 material_colour;
// Shininess of the object
uniform float shininess;
// Light colour
uniform vec4 light_colour;
// Direction of the light
uniform vec3 light_dir;
// Position of the camera
uniform vec3 eye_pos;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 2) in vec3 normal;

// Outgoing vertex colour
layout(location = 0) out vec4 vertex_colour;

void main() {
  // *********************************
  // Calculate position
  gl_Position = MVP * vec4(position, 1.0);
  // Transform the normal
  vec3 transformed_normal = N * normal;
  // Calculate world position
  vec3 world_pos = vec3(M * vec4(position, 1.0f));
  // Calculate view direction
  vec3 view_dir = normalize(vec3(eye_pos - world_pos));
  // Calculate half vector between view_dir and light_dir
  vec3 H = normalize(light_dir + view_dir);
  // Calculate k
  float k = pow(max(dot(transformed_normal, H), 0.0), shininess);
  // Calculate specular
  vec4 specular = k * (light_colour * material_colour);
  // Ensure alpha is 1
  specular.a = 1.0f;
  // Output vertex colour - just specular
  vertex_colour = specular;
  // *********************************
}