#version 440

// The transformation matrix
uniform mat4 MVP;
// The ambient intensity of the scene
uniform vec4 ambient_intensity;
// The material colour of the object
uniform vec4 material_colour;

// Incoming position data
layout(location = 0) in vec3 position;

// The outgoing vertex colour
layout(location = 0) out vec4 vertex_colour;

void main() {
  // *********************************
  // Calculate position
  vec4 v = vec4(position,1.0);
  gl_Position = MVP * v;
  // Calculate ambient component
   vertex_colour = material_colour * ambient_intensity;
  // Output vertex colour
  
  // *********************************
}


  /*float red = material_colour.r * ambient_intensity.r;
  float green = material_colour.g * ambient_intensity.g;
  float blue = material_colour.b * ambient_intensity.b;
  float alpha = material_colour.a * ambient_intensity.a;

  vec4 grant = vec4(red, green, blue, alpha);
  vertex_colour = grant;*/