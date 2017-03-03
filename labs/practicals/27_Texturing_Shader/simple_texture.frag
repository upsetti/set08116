#version 440

out vec4 daColour;
in vec3 theNormal
in vec3 thePosition;

uniform vec3 lightPosition;


// Incoming vertex colour
layout(location = 0) in vec4 vertex_colour;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main()
{
	vec3 lightVector = normalize(lightPosition - thePosition);
	float brightness = dot(lightVector, theNormal);
	daColour = vec4(brightness, brightness, brightness, 1.0);
	// Set outgoing vertex colour
   colour = vertex_colour;
}

