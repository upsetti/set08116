#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

mesh water;
effect water_eff;
target_camera cam;
// Main texture
texture watertex;
// Dissolve texture
//texture dissolve;
// Dissolve factor to set on shader
//float dissolve_factor = 1.0f;
vec2 uv_scroll;

bool load_content() {
  // Create mesh object, cheating and using the mesh builder for now
  water = mesh(geometry_builder::create_plane(25,25));
  // Scale geometry
  water.get_transform().scale = vec3(10.0f);

  // Load in dissolve shader
  water_eff.add_shader("shaders/dissolve.vert", GL_VERTEX_SHADER);
  water_eff.add_shader("shaders/dissolve.frag", GL_FRAGMENT_SHADER);

  // Build effect
  water_eff.build();

  // Load in textures
  watertex = texture("textures/watertex.jpg");
 // dissolve = texture("textures/watertex.jpg");

  // Set camera properties
  cam.set_position(vec3(30.0f, 30.0f, 30.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  auto aspect = static_cast<float>(renderer::get_screen_width()) / static_cast<float>(renderer::get_screen_height());
  cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 1000.0f);

  return true;
}

bool update(float delta_time) {
  // Use up an down to modify the dissolve factor
  //if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP))
 //   dissolve_factor = clamp(dissolve_factor + 0.1f * delta_time, 0.0f, 1.0f);
 // if (glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN))
  //  dissolve_factor = clamp(dissolve_factor - 0.1f * delta_time, 0.0f, 1.0f);
  // Update camera
  cam.update(delta_time);
  uv_scroll += vec2(0, delta_time * 0.05);
  return true;
}

bool render() {
  // Bind effect
  renderer::bind(water_eff);

  // Create MVP matrix
  auto M = water.get_transform().get_transform_matrix();
  auto V = cam.get_view();
  auto P = cam.get_projection();
  auto MVP = P * V * M;

  // Set MVP matrix uniform
  glUniformMatrix4fv(water_eff.get_uniform_location("MVP"), // Location of uniform
                     1,                               // Number of values - 1 mat4
                     GL_FALSE,                        // Transpose the matrix?
                     value_ptr(MVP));                 // Pointer to matrix data

  // *********************************
  // Set the dissolve_factor uniform value

  // Bind the two textures - use different index for each


  // Set the uniform values for textures - use correct index


  // *********************************

  // Set UV_scroll uniform, adds cool movent (Protip: This is a super easy way to do fire effects;))
  glUniform2fv(water_eff.get_uniform_location("UV_SCROLL"), 1, value_ptr(uv_scroll));
  // Render the mesh
  renderer::render(water);

  return true;
}

void main() {
  // Create application
  app application("33_Dissolve");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}