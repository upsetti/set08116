#include <glm/glm.hpp>
//#include <glm/gtx/rotate_vector.hpp>
#include <graphics_framework.h>
using namespace std;
using namespace graphics_framework;
using namespace glm;

mesh sphere;
mesh skybox;
mesh ground;
effect eff;
effect sky_eff;
cubemap cube_map;
free_camera cam;
texture sand;

double cursor_x = 0.0;
double cursor_y = 0.0;

bool initialise() {
	// *********************************
	// Set input mode - hide the cursor
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Capture initial mouse position
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	// *********************************
	return true;
}

bool load_content() {
	// Create a sphere
	sphere = mesh(geometry_builder::create_sphere(25, 25));
	ground = mesh(geometry_builder::create_plane(100, 100));
	// *********************************
	// Create box geometry for skybox
	skybox = mesh(geometry_builder::create_box());
	// Scale box by 100
	skybox.get_transform().scale = vec3(200.0f, 200.0f, 200.0f);
	// Load the cubemap
	 array<string, 6> filenames = {"textures/box_right.jpg", "textures/box_left.jpg", "textures/box_top.jpg",
                                "textures/box_bottom.jpg", "textures/box_front.jpg", "textures/box_back.jpg"};
  cube_map = cubemap(filenames);
	// *********************************
  //texture
  sand = texture("textures/sand.jpg");

  // Load in shaders
	eff.add_shader("57_Skybox/shader.vert", GL_VERTEX_SHADER);
	eff.add_shader("57_Skybox/shader.frag", GL_FRAGMENT_SHADER);
	// Build effect
	eff.build();

	// *********************************
	// Load in skybox effect
	sky_eff.add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
	sky_eff.add_shader("shaders/skybox.frag", GL_FRAGMENT_SHADER);
	// Build effect
	sky_eff.build();
	// *********************************

	// Set camera properties
	// Set camera properties
	cam.set_position(vec3(0.0f, 10.0f, 0.0f));
	cam.set_target(vec3(0.0f, 0.0f, 0.0f));
	auto aspect = static_cast<float>(renderer::get_screen_width()) / static_cast<float>(renderer::get_screen_height());
	cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 1000.0f);
	return true;
}
float theta;
bool update(float delta_time) {
	// The ratio of pixels to rotation - remember the fov
	static double ratio_width = pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height = (pi<float>() * (static_cast<float>(renderer::get_screen_height()) /
		static_cast<float>(renderer::get_screen_width()))) /
		static_cast<float>(renderer::get_screen_height());

	double current_x;
	double current_y;
	// *********************************
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;
	// Multiply deltas by ratios and delta_time - gets actual change in orientation
	delta_x = delta_x * ratio_width;
	delta_y = delta_y * ratio_height;
	// Rotate cameras by delta
	// delta_y - x-axis rotation
	// delta_x - y-axis rotation
	cam.rotate(delta_x, -delta_y);
	// Use keyboard to move the camera - WSAD
	vec3 translation(0.0f, 0.0f, 0.0f);
	if (glfwGetKey(renderer::get_window(), 'W')) {
		translation.z += 10.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'A')) {
		translation.x -= 10.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'S')) {
		translation.z -= 10.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'D')) {
		translation.x += 10.0f * delta_time;
	}
	// Move camera
	cam.move(translation);
	// Update the camera
	cam.update(delta_time);
	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;
	// *********************************
	// Set skybox position to camera position (camera in centre of skybox)
	skybox.get_transform().position = vec3(15.0f, 5.0f, 15.0f);
	// *********************************
	return true;
}

bool render() {
	// *********************************
	// Disable depth test,depth mask,face culling
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	// Bind skybox effect
	renderer::bind(sky_eff);
	// Calculate MVP for the skybox
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V * scale(mat4(), vec3(100.0f));

	// Set MVP matrix uniform
	glUniformMatrix4fv(sky_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

	// Set cubemap uniform
	renderer::bind(cube_map, 0);
	glUniform1i(sky_eff.get_uniform_location("cubemap"), 0);

	// Render skybox
	renderer::render(skybox);

	// Enable depth test,depth mask,face culling
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);


	// *********************************

	// Bind effect
	renderer::bind(eff);
	// Create MVP matrix
	mat4 M = sphere.get_transform().get_transform_matrix();
	MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Render mesh

	renderer::bind(sand, 0);
	glUniform1i(eff.get_uniform_location("sand"), 0);
	renderer::render(sphere);


	return true;
}

void main() {
	// Create application
	app application("57_Skybox");
	// Set methods
	application.set_load_content(load_content);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}