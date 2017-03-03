#version 440

// *********************************
// Define the output colour for the shader here
uniform vec4 colour;
layout (location = 0) in vec4 colour_in;
// *********************************

// Outgoing colour for the shader
layout(location = 0) out vec4 out_colour;

void main() {
  // *********************************
  // Set outgoing colour
  out_colour = colour;
  // *********************************
} 