#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

map<string, mesh> meshes;
effect eff;
texture tex;
target_camera targetcam;
GLuint programID;

bool load_content() {
  // Create plane mesh
  meshes["plane"] = mesh(geometry_builder::create_plane());

  // Create scene
  meshes["box"] = mesh(geometry_builder::create_box());
  meshes["tetra"] = mesh(geometry_builder::create_tetrahedron());
  meshes["pyramid"] = mesh(geometry_builder::create_pyramid());
  meshes["disk"] = mesh(geometry_builder::create_disk(20));
  meshes["cylinder"] = mesh(geometry_builder::create_cylinder(20, 20));
  meshes["sphere"] = mesh(geometry_builder::create_sphere(20, 20));
  meshes["torus"] = mesh(geometry_builder::create_torus(20, 20, 1.0f, 5.0f));

  // Transform objects
  meshes["box"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["box"].get_transform().translate(vec3(-10.0f, 2.5f, -30.0f));
  meshes["tetra"].get_transform().scale = vec3(4.0f, 4.0f, 4.0f);
  meshes["tetra"].get_transform().translate(vec3(-30.0f, 10.0f, -10.0f));
  meshes["pyramid"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["pyramid"].get_transform().translate(vec3(-10.0f, 7.5f, -30.0f));
  meshes["disk"].get_transform().scale = vec3(3.0f, 1.0f, 3.0f);
  meshes["disk"].get_transform().translate(vec3(-10.0f, 11.5f, -30.0f));
  meshes["disk"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
  meshes["cylinder"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["cylinder"].get_transform().translate(vec3(-25.0f, 2.5f, -25.0f));
  meshes["sphere"].get_transform().scale = vec3(2.5f, 2.5f, 2.5f);
  meshes["sphere"].get_transform().translate(vec3(-25.0f, 10.0f, -25.0f));
  meshes["torus"].get_transform().translate(vec3(-25.0f, 10.0f, -25.0f));
  meshes["torus"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));

  // Load texture
  tex = texture("textures/checker.png");

  // Load in shaders
  eff.add_shader("27_Texturing_Shader/simple_texture.vert", GL_VERTEX_SHADER);
  eff.add_shader("27_Texturing_Shader/simple_texture.frag", GL_FRAGMENT_SHADER);

  // Build effect
  eff.build();

  // Set camera properties
  targetcam.set_position(vec3(50.0f, 10.0f, 50.0f));
  targetcam.set_target(vec3(0.0f, 0.0f, 0.0f));
  targetcam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
  return true;
}

bool update(float delta_time) {
  // *********************************
  // Use keyboard to change camera location
  // 1 - (50, 10, 50)
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_1)) {
		targetcam.set_position(vec3(50.0f, 10.0f, 50.0f));
	}
  // 2 - (-50, 10, 50)
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_2)) {
		targetcam.set_position(vec3(-50.0f, 10.0f, 50.0f));
	}
  // 3 - (-50, 10, -50)
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_3)) {
		targetcam.set_position(vec3(-50.0f, 10.0f, -50.0f));
	}
  // 4 - (50, 10, -50)
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_4)) {
		targetcam.set_position(vec3(50.0f, 10.0f, -50.0f));
	}

  // Update the camera
	targetcam.update(delta_time);
  // *********************************
  /*mat4 fullTransformMatrix;
  GLint ambientLightUniformLocation = glGetUniformLocation(programID, "ambientLight");
  vec3 ambientLight(0.1f, 0.1f, 0.1f);
  glUniform3fv(ambientLightUniformLocation, 1, &ambientLight[0]);
  GLint lightPositionUniformLocation = glGetUniformLocation(programID, "lightPosition");
  glm::vec3 lightPosition(0.0f, 1.0f, 0.0f);
  glUniform3fv(lightPositionUniformLocation, 1, &lightPosition[0]);
  */
  return true;
}

bool render() {
  // Render meshes
  for (auto &e : meshes) {
    auto m = e.second;
    // Bind effect
    renderer::bind(eff);
    // Create MVP matrix
    auto M = m.get_transform().get_transform_matrix();
    auto V = targetcam.get_view();
    auto P = targetcam.get_projection();
    auto MVP = P * V * M;
    // Set MVP matrix uniform
    glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

    // Bind and set texture
    renderer::bind(tex, 0);
    glUniform1i(eff.get_uniform_location("tex"), 0);

    // Render mesh
    renderer::render(m);
  }

  return true;
}

void main() {
  // Create application
  app application("37_Target_Camera");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}