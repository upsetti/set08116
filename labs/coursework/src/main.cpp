#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

map<string, mesh> meshes;
map<string, texture> textures;
mesh skybox;
effect eff;
effect sky_eff;
texture tex;
free_camera freecam;
target_camera targetcam;
GLuint programID;
bool Fcam = false;
cubemap cube_map;
double cursor_x = 0.0;
double cursor_y = 0.0;
float theta = 0.0f;
vector<point_light> points(4);
vector<spot_light> spots(5);

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

	//set materials
	material mat;
	mat.set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat.set_diffuse(vec4(1.0f, 0.0f, 0.0f, 1.0f));
	mat.set_shininess(25.0f);

	material mat2;
	mat2.set_emissive(vec4(0.3f, 0.4f, 0.3f, 1.0f));
	mat2.set_specular(vec4(2.0f, 2.0f, 2.0f, 1.0f));
	mat2.set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat2.set_shininess(40.0f);

	// Create enviroment
	meshes["plane"] = mesh(geometry_builder::create_plane(65, 65));
	textures["plane"] = texture("textures/darksand.jpg");

	skybox = mesh(geometry_builder::create_box());

	//trees
	meshes["canopy"] = mesh(geometry_builder::create_pyramid());
	meshes["canopy"].get_transform().scale = vec3(5.0f, 10.0f, 5.0f);
	meshes["canopy"].get_transform().translate(vec3(-23.0f, 7.5f, -20.0f));
	mat.set_diffuse(vec4(0.0f, 1.0f, 0.0f, 1.0f));
	meshes["canopy"].set_material(mat);
	textures["canopy"] = texture("textures/canopy.png");

	meshes["trunk"] = mesh(geometry_builder::create_cylinder(20, 20));
	meshes["trunk"].get_transform().scale = vec3(2.5f, 5.0f, 2.5f);
	meshes["trunk"].get_transform().translate(vec3(-23.0f, 2.5f, -20.0f));
	mat.set_diffuse(vec4(1.0f, 0.6f, 0.2f, 1.0f));
	meshes["trunk"].set_material(mat);
	textures["trunk"] = texture("textures/trunk.png");

	meshes["canopy2"] = mesh(geometry_builder::create_pyramid());
	meshes["canopy2"].get_transform().scale = vec3(5.0f, 10.0f, 5.0f);
	meshes["canopy2"].get_transform().translate(vec3(-27.0f, 7.5f, -17.0f));
	mat2.set_diffuse(vec4(0.0f, 0.5f, 0.0f, 1.0f));
	meshes["canopy2"].set_material(mat2);
	textures["canopy2"] = texture("textures/canopy.png");

	meshes["trunk2"] = mesh(geometry_builder::create_cylinder(20, 20));
	meshes["trunk2"].get_transform().scale = vec3(2.5f, 5.0f, 2.5f);
	meshes["trunk2"].get_transform().translate(vec3(-27.0f, 2.5f, -17.0f));
	mat.set_diffuse(vec4(1.0f, 0.6f, 0.2f, 1.0f));
	meshes["trunk2"].set_material(mat);
	textures["trunk2"] = texture("textures/trunk.png");

	//rock
	geometry geom1("models/stone_2.obj");
	meshes["mountains"] = mesh(geom1);
	meshes["mountains"].get_transform().translate(vec3(25.0f, 0.0f, 15.0f));
	mat.set_diffuse(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	meshes["mountains"].set_material(mat);
	textures["mountains"] = texture("textures/Craggy_Rock_With_Moss_UV_CM_1.jpg");

	//alien
	geometry geom2("models/alien.obj");
	meshes["marvin"] = mesh(geom2);
	meshes["marvin"].get_transform().translate(vec3(25.0f, 5.0f, 15.0f));
	meshes["marvin"].get_transform().rotate(vec3(0.0f, theta -= half_pi<float>(), 0.0f));
	mat.set_diffuse(vec4(1.0f, 0.0f, 0.0f, 1.0f));
	meshes["marvin"].set_material(mat);
	textures["marvin"] = texture("textures/check_2.png");

	//sphere
	meshes["orb"] = mesh(geometry_builder::create_sphere(20, 20));
	meshes["orb"].get_transform().scale = vec3(0.5f, 0.5f, 0.5f);
	meshes["orb"].get_transform().translate(vec3(0.0f, 10.0f, 0.0f));
	mat2.set_diffuse(vec4(0.7f, 0.7f, 0.7f, 1.0f));
	meshes["orb"].set_material(mat2);
	textures["orb"] = texture("textures/universe.jpg");
	//torus
	meshes["torus1"] = mesh(geometry_builder::create_torus(20, 20, 1.0f, 5.0f));
	meshes["torus1"].get_transform().translate(vec3(0.0f, 10.0f, 0.0f));
	meshes["torus1"].get_transform().scale = vec3(0.25f, 0.25f, 0.25f);
	mat.set_diffuse(vec4(1.0f, 2.0f, 2.0f, 1.0f));
	meshes["torus1"].set_material(mat);
	textures["torus1"] = texture("textures/geo.jpg");

	meshes["torus2"] = mesh(geometry_builder::create_torus(20, 20, 1.0f, 5.0f));
	meshes["torus2"].get_transform().translate(vec3(0.0f, 10.0f, 0.0f));
	meshes["torus2"].get_transform().scale = vec3(0.5f, 0.5f, 0.5f);
	mat.set_diffuse(vec4(1.5f, 1.0f, 0.8f, 1.0f));
	meshes["torus2"].set_material(mat);
	textures["torus2"] = texture("textures/colour.jpg");

	meshes["torus3"] = mesh(geometry_builder::create_torus(20, 20, 1.0f, 5.0f));
	meshes["torus3"].get_transform().translate(vec3(0.0f, 10.0f, 0.0f));
	meshes["torus3"].get_transform().scale = vec3(0.75f, 0.75f, 0.75f);
	mat.set_diffuse(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	meshes["torus3"].set_material(mat);
	textures["torus3"] = texture("textures/lamecolour.jpg");

	//orbits
	meshes["lexicon"] = mesh(geometry_builder::create_box());
	meshes["lexicon"].get_transform().translate(vec3(0.0f, 10.0f, -9.0f));
	mat.set_diffuse(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	meshes["lexicon"].set_material(mat);
	textures["lexicon"] = texture("textures/yell.png");

	meshes["pyramid"] = mesh(geometry_builder::create_pyramid());
	meshes["pyramid"].get_transform().translate(vec3(0.0f, 10.0f, 9.0f));
	mat.set_diffuse(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	meshes["pyramid"].set_material(mat);
	textures["pyramid"] = texture("textures/pyramid.jpg");

	//moon
	meshes["luna"] = mesh(geometry_builder::create_sphere(100, 100));
	meshes["luna"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
	meshes["luna"].get_transform().translate(vec3(25.0f, 5.0f, -25.0f));
	mat.set_diffuse(vec4(2.0f, 2.0f, 2.0f, 1.0f));
	mat.set_shininess(15.0f);
	meshes["luna"].set_material(mat);
	textures["luna"] = texture("textures/moonmap2k.jpg");

	// Set lighting values
	points[0].set_position(vec3(-25.0f, 70.0f, 25.0f));
	points[0].set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	points[0].set_range(70.0f);

	points[1].set_position(vec3(25.0f, 12.0f, 15.0f));
	points[1].set_light_colour(vec4(0.5f, 0.0f, 0.0f, 1.0f));
	points[1].set_range(13.0f);

	spots[1].set_position(vec3(0.0f, 15.0f, 0.0f));
	spots[1].set_light_colour(vec4(2.0f, 2.0f, 2.0f, 1.0f));
	spots[1].set_direction(normalize(vec3(0.0f, -1.0f, 0.0f)));
	spots[1].set_range(30.0f);
	spots[1].set_power(3.0f);

	spots[2].set_position(vec3(-10.0f, 10.0f, -15.0f));
	spots[2].set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	spots[2].set_direction(normalize(vec3(-1.0f, -1.0f, -1.0f)));
	spots[2].set_range(20.0f);
	spots[2].set_power(1.5f);

	// skybox
	skybox.get_transform().scale = vec3(100.0f, 100.0f, 100.0f);
	array<string, 6> filenames = { "textures/face.png", "textures/back.png", "textures/up.png",
		"textures/down.png", "textures/left.png", "textures/right.png" };
	cube_map = cubemap(filenames);

	// Load in shaders
	eff.add_shader("shaders/multi-light.vert", GL_VERTEX_SHADER);
	eff.add_shader("shaders/multi-light.frag", GL_FRAGMENT_SHADER);
	// Build effect
	eff.build();

	sky_eff.add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
	sky_eff.add_shader("shaders/skybox.frag", GL_FRAGMENT_SHADER);
	// Build effect
	sky_eff.build();

	// Set camera properties
	targetcam.set_position(vec3(-40.0f, 3.0f, 40.0f));
	targetcam.set_target(vec3(0.0f, 12.0f, 0.0f));
	targetcam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 2.414f, 1000.0f);

	freecam.set_position(vec3(pi<float>() / 6, 10.0f, 20.0f));
	freecam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 2.414f, 1000.0f);

	return true;

}

bool update(float delta_time) {



	// Use keyboard to move the camera - WSAD
	vec3 translation(0.0f, 0.0f, 0.0f);
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_W)) {
		translation.z += 20.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_A)) {
		translation.x -= 20.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_S)) {
		translation.z -= 20.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_D)) {
		translation.x += 20.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_R)) {
		translation.y += 20.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_E)) {
		translation.y -= 20.0f * delta_time;
	}
	// Use keyboard to change camera location
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_1)) {
		Fcam = false;
		targetcam.set_position(vec3(-13.0f, 9.0f, 13.0f));
		targetcam.set_target(vec3(0.0f, 10.0f, 0.0f));
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_2)) {
		Fcam = false;
		targetcam.set_position(vec3(40.0f, 5.0f, -40.0f));
		targetcam.set_target(vec3(0.0f, 21.0f, 0.0f));
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_3)) {
		Fcam = false;
		targetcam.set_position(vec3(-25.0f, 70.0f, 25.0f));
		targetcam.set_target(vec3(0.0f, 10.0f, 0.0f));
	}

	//free cam
	// The ratio of pixels to rotation - remember the fov
	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height = (quarter_pi<float>() * (static_cast<float>(renderer::get_screen_height()) /
		static_cast<float>(renderer::get_screen_width()))) /
		static_cast<float>(renderer::get_screen_height());

	double current_x;
	double current_y;
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;
	// Change in orientation
	delta_x = delta_x * ratio_width;
	delta_y = delta_y * ratio_height;
	// Rotate cameras
	freecam.rotate(delta_x, -delta_y);

	if (glfwGetKey(renderer::get_window(), GLFW_KEY_F)) {
		Fcam = true;
		freecam.set_target(targetcam.get_target());
		freecam.set_position(freecam.get_position());
	}

	// Move camera
	freecam.move(translation);
	// Update the camera
	freecam.update(delta_time);
	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;

	targetcam.update(delta_time);

	//move skybox
	skybox.get_transform().position = vec3(15.0f, 5.0f, 15.0f);

	theta += pi<float>() * delta_time / 2;
	//Geometry
	meshes["orb"].get_transform().rotate(vec3(delta_time / 2, delta_time / 2, delta_time / 2));
	meshes["torus1"].get_transform().rotate(vec3(-delta_time * 2, -delta_time * 2, -delta_time * 2));
	meshes["torus2"].get_transform().rotate(vec3(0.0f, delta_time, delta_time));
	meshes["torus3"].get_transform().rotate(vec3(-delta_time / 3, -delta_time / 3, 0.0f));
	meshes["lexicon"].get_transform().rotate(vec3(delta_time / 3, -delta_time / 3, 0.0f));
	meshes["lexicon"].get_transform().position.x += sin(theta) * 0.2;
	meshes["lexicon"].get_transform().position.z += cos(theta) * 0.2;
	meshes["lexicon"].get_transform().position.y -= sin(theta) * 0.1;
	meshes["pyramid"].get_transform().rotate(vec3(-delta_time * 2, -delta_time * 2, 0.0f));
	meshes["pyramid"].get_transform().position.x += sin(theta) * 0.2;
	meshes["pyramid"].get_transform().position.z -= cos(theta) * 0.2;
	meshes["pyramid"].get_transform().position.y += sin(theta) * 0.1;
	meshes["luna"].get_transform().rotate(vec3(-delta_time / 3, -delta_time / 3, 0.0f));

	return true;
}

bool render() {
	// Render meshes
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	// Bind skybox effect
	renderer::bind(sky_eff);
	// Calculate MVP for the skybox
	auto V = targetcam.get_view();//freecam.get_view();
	auto P = targetcam.get_projection();//freecam.get_projection();
	if (Fcam) {
		V = freecam.get_view();
		P = freecam.get_projection();
	}
	auto MVP = P * V * scale(mat4(), vec3(500.0f));

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


	for (auto &e : meshes) {
		auto m = e.second;
		// Bind effect
		renderer::bind(eff);
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		//Camera type
		auto V = targetcam.get_view();//freecam.get_view();
		auto P = targetcam.get_projection();//freecam.get_projection();
		if (Fcam) {
			V = freecam.get_view();
			P = freecam.get_projection();
		}
		auto MVP = P * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

		glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
		// Set N matrix uniform - remember - 3x3 matrix
		glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
		// Bind material
		renderer::bind(m.get_material(), "mat");
		renderer::bind(m.get_material(), "mat2");
		// Bind point lights
		renderer::bind(points, "points");
		// Bind spot lights
		renderer::bind(spots, "spots");

		// Bind and set texture
		renderer::bind(textures[e.first], 0);
		glUniform1i(eff.get_uniform_location("tex"), 0);

		// Set eye position- Get this from active camera
		glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(targetcam.get_position()));

		// Render mesh
		renderer::render(m);
	}

	return true;
}

void main() {
	// Create application
	app application("Graphics Coursework");
	// Set load content, update and render methods
	application.set_load_content(load_content);
	application.set_initialise(initialise);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}