#version 440

// Point light information
struct point_light {
  vec4 light_colour;
  vec3 position;
  float constant;
  float linear;
  float quadratic;
};

// Material information
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};

// Point light for the scene
uniform point_light point;
// Material for the object
uniform material mat;
// Eye position
uniform vec3 eye_pos;
// Texture
uniform sampler2D tex;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 1) in vec3 normal;
// Incoming texture coordinate
layout(location = 2) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Get distance between point light and vertex
  float d = distance(point.position, position);
  // Calculate attenuation factor
  float k_att = point.constant + (point.linear * d) + (point.quadratic * d * d);
  // Calculate light colour
  vec4 light_colour = point.light_colour / k_att;
  light_colour.a = 1.0f;
  // Calculate light dir
  vec3 light_dir = normalize(point.position - position);
  // Now use standard phong shading but using calculated light colour and direction
  // - note no ambient

  // Calculate diffuse component
  float kd = max(dot(normal, light_dir), 0.0);
  vec4 diffuse = kd * (mat.diffuse_reflection * light_colour);
  // Calculate view direction
  vec3 view_dir = normalize(vec3(eye_pos - position));
  // Calculate half vector
  vec3 H = normalize(light_dir + view_dir);
  // Calculate specular component
  float ks = pow(max(dot(H, normal), 0.0), mat.shininess);
  vec4 specular = ks * (light_colour * mat.specular_reflection);
  // Sample texture
  vec4 tex_colour = texture(tex, tex_coord);
  // Calculate primary colour component
  vec4 primary = mat.emissive + diffuse;
  vec4 secondary = specular;
  // Calculate final colour - remember alpha
  primary.a = 1;
  secondary.a = 1;
  colour = primary * tex_colour + secondary;

  // *********************************
}