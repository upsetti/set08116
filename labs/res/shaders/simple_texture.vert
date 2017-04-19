#version 440

in layout(location=0) vec3 position;
in layout(location=1) vec3 vertexColour;
in layout(location=2) vec3 normal;
uniform vec3 ambientLight;
uniform mat4 fullTransformMatrix;
out vec3 theNormal;
out vec3 thePosition;

// The transformation matrix
uniform mat4 MVP;
// The normal matrix
uniform mat3 N;
// Material colour
uniform vec4 material_colour;
// Light colour
uniform vec4 light_colour;
// Direction of the light
uniform vec3 light_dir;
// Incoming position
layout(location = 3) in vec3 position;
// Incoming normal
layout(location = 4) in vec3 normal;
// Outgoing vertex colour
layout(location = 0) out vec4 vertex_colour;

void main() {
	vec4 v = vec4(position, 1.0);
	gl_position = fullTransformationMatrix * v;
	theNormal = normal;
	thePosition = position;

	  // Calculate position
  gl_Position = MVP * vec4(position,1.0);
  // Calculate diffuse component - use transformed normal
  vec3 transformed_normal = N * normal;
  // *********************************

  // Calculate k
  float k = max(dot(transformed_normal, light_dir), 0.0);
  // Calculate diffuse
  vec4 diffuse = k * (material_colour * light_colour);

  // Ensure alpha is 1
  diffuse.a = 1.0;

  // Output vertex colour - just diffuse
  vertex_colour = diffuse;
}

