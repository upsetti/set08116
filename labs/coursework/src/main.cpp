#include <glm/glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

//terrain
mesh terr;
effect eff;
texture tex[4];
directional_light light;
//skybox
mesh skybox;
effect sky_eff;
cubemap cube_map;
//moon
mesh moon;
effect moon_eff;
texture moon_tex;
texture moon_norm_tex;
directional_light lunarlight;
//water
mesh water;
effect water_eff;
texture watertex;
vec2 uv_scroll;
directional_light waterlight;
//flames
mesh fire;
texture fire_tex;
effect fire_eff;
texture fire_dissolve;
vec2 uv_scroll2;
float dissolve_factor = 1.0f;
vector<spot_light> flames(5);
//geometry
effect geom_eff;
map<string, mesh> meshes;
map<string, texture> textures;
vector<point_light> points(4);
vector<spot_light> spots(5);

//misc
free_camera freecam;
target_camera targetcam;
bool Fcam = false;
float theta = 0.0f;
double cursor_x = 0.0;
double cursor_y = 0.0;

bool initialise() {
	// hide the cursor / get initial mouse position
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	return true;
}

void generate_terrain(geometry &geom, const texture &height_map, unsigned int width, unsigned int depth,
	float height_scale) {
	vector<vec3> positions;
	vector<vec3> normals;
	vector<vec2> tex_coords;
	vector<vec4> tex_weights;
	vector<unsigned int> indices;

	// Extract the texture data from the image
	glBindTexture(GL_TEXTURE_2D, height_map.get_id());
	auto data = new vec4[height_map.get_width() * height_map.get_height()];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, (void *)data);
	// Determine ratio of height map to geometry
	float width_point = static_cast<float>(width) / static_cast<float>(height_map.get_width());
	float depth_point = static_cast<float>(depth) / static_cast<float>(height_map.get_height());
	// Point to work on
	vec3 point;
	// find calculate vertex and add to vector for each point
	for (int x = 0; x < height_map.get_width(); ++x) {
		point.x = -(width / 2.0f) + (width_point * static_cast<float>(x));

		for (int z = 0; z < height_map.get_height(); ++z) {
			point.z = -(depth / 2.0f) + (depth_point * static_cast<float>(z));
			point.y = data[(z * height_map.get_width()) + x].y * height_scale;
			positions.push_back(point);
		}
	}

	// index data
	for (unsigned int x = 0; x < height_map.get_width() - 1; ++x) {
		for (unsigned int y = 0; y < height_map.get_height() - 1; ++y) {
			unsigned int top_left = (y * height_map.get_width()) + x;
			unsigned int top_right = (y * height_map.get_width()) + x + 1;
			unsigned int bottom_left = ((y + 1) * height_map.get_width()) + x;
			unsigned int bottom_right = ((y + 1) * height_map.get_height()) + x + 1;
			indices.push_back(top_left);
			indices.push_back(bottom_right);
			indices.push_back(bottom_left);
			indices.push_back(top_left);
			indices.push_back(top_right);
			indices.push_back(bottom_right);
		}
	}

	// Resize the normals buffer
	normals.resize(positions.size());
	// find normals from height map
	for (unsigned int i = 0; i < indices.size() / 3; ++i) {
		auto idx1 = indices[i * 3];
		auto idx2 = indices[i * 3 + 1];
		auto idx3 = indices[i * 3 + 2];
		vec3 side1 = positions[idx1] - positions[idx3];
		vec3 side2 = positions[idx1] - positions[idx2];
		vec3 n = cross(side2, side1);
		normals[idx1] = normals[idx1] + n;
		normals[idx2] = normals[idx2] + n;
		normals[idx3] = normals[idx3] + n;
	}

	// Normali
	for (auto &n : normals) {
		n = normalize(n);
	}
	// texture coordinates
	for (unsigned int x = 0; x < height_map.get_width(); ++x) {
		for (unsigned int z = 0; z < height_map.get_height(); ++z) {
			tex_coords.push_back(vec2(width_point * x, depth_point * z));
		}
	}
	// texture weight
	for (unsigned int x = 0; x < height_map.get_width(); ++x) {
		for (unsigned int z = 0; z < height_map.get_height(); ++z) {
			vec4 tex_weight(clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.0f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.15f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.5f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.9f) / 0.25f, 0.0f, 1.0f));

			float total = tex_weight.x + tex_weight.y + tex_weight.z + tex_weight.a;
			tex_weight = tex_weight / total;
			tex_weights.push_back(tex_weight);
		}
	}
	// terrain buffers
	geom.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	geom.add_buffer(normals, BUFFER_INDEXES::NORMAL_BUFFER);
	geom.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	geom.add_buffer(tex_weights, BUFFER_INDEXES::TEXTURE_COORDS_1);
	geom.add_index_buffer(indices);
	delete[] data;
}

bool load_content() {

	//materials
	material moonmat;
	moonmat.set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	moonmat.set_specular(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	moonmat.set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	moonmat.set_shininess(15.0);

	material watermat;
	watermat.set_diffuse(vec4(0.0f, 0.0f, 1.0f, 1.0f));
	water.get_material().set_diffuse(vec4(0.0f, 0.0f, 1.0f, 1.0f));
	watermat.set_shininess(15.0);

	// terrain
	geometry geom;
	texture height_map("textures/hmap.png");
	generate_terrain(geom, height_map, 20, 20, 2.0f);
	terr = mesh(geom);

	skybox = mesh(geometry_builder::create_box());
	// skybox
	skybox.get_transform().scale = vec3(100.0f, 100.0f, 100.0f);
	array<string, 6> filenames = { "textures/face.png", "textures/back.png", "textures/up.png",
		"textures/down.png", "textures/left.png", "textures/right.png" };
	cube_map = cubemap(filenames);
	sky_eff.add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
	sky_eff.add_shader("shaders/skybox.frag", GL_FRAGMENT_SHADER);
	sky_eff.build();

	// geometry
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

	// water
	water = mesh(geometry_builder::create_plane(20, 20));
	water.get_transform().translate(vec3(0.0f, 0.20f, 0.0f));
	water_eff.add_shader("shaders/dissolve.vert", GL_VERTEX_SHADER);
	water_eff.add_shader("shaders/dissolve.frag", GL_FRAGMENT_SHADER);
	water_eff.build();

	// moon / normal mapping
	moon = mesh(geometry_builder::create_sphere(100, 100));
	moon.get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
	moon.get_transform().translate(vec3(30.0f, 20.0f, 30.0f));
	moon.set_material(moonmat);
	moon_tex = texture("textures/moonmap2k.jpg");
	moon_norm_tex = texture("textures/moonnormalmap.png");
	lunarlight.set_ambient_intensity(vec4(0.3f, 0.3f, 0.3f, 1.0f));
	lunarlight.set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	lunarlight.set_direction(normalize(vec3(-1.1f, 1.0f, -1.0f)));
	moon_eff.add_shader("shaders/shader.vert", GL_VERTEX_SHADER);
	moon_eff.add_shader("shaders/shader.frag", GL_FRAGMENT_SHADER);
	moon_eff.add_shader("shaders/part_direction.frag", GL_FRAGMENT_SHADER);
	moon_eff.add_shader("shaders/part_normal_map.frag", GL_FRAGMENT_SHADER);
	moon_eff.build();

	// geometry
	meshes["orb"] = mesh(geometry_builder::create_sphere(20, 20));
	meshes["orb"].get_transform().scale = vec3(0.05f, 0.05f, 0.05f);
	meshes["orb"].get_transform().translate(vec3(-8.0, 2.5f, -8.0f));
	textures["orb"] = texture("textures/universe.jpg");
	meshes["torus1"] = mesh(geometry_builder::create_torus(20, 20, 1.0f, 5.0f));
	meshes["torus1"].get_transform().translate(vec3(-8.0, 2.5f, -8.0f));
	meshes["torus1"].get_transform().scale = vec3(0.025f, 0.025f, 0.025f);
	mat.set_diffuse(vec4(1.0f, 2.0f, 2.0f, 1.0f));
	meshes["torus1"].set_material(mat);
	textures["torus1"] = texture("textures/geo.jpg");
	meshes["torus2"] = mesh(geometry_builder::create_torus(20, 20, 1.0f, 5.0f));
	meshes["torus2"].get_transform().translate(vec3(-8.0, 2.5f, -8.0f));
	meshes["torus2"].get_transform().scale = vec3(0.05f, 0.05f, 0.05f);
	mat.set_diffuse(vec4(1.5f, 1.0f, 0.8f, 1.0f));
	meshes["torus2"].set_material(mat);
	textures["torus2"] = texture("textures/colour.jpg");
	meshes["torus3"] = mesh(geometry_builder::create_torus(20, 20, 1.0f, 5.0f));
	meshes["torus3"].get_transform().translate(vec3(-8.0, 2.5f, -8.0f));
	meshes["torus3"].get_transform().scale = vec3(0.075, 0.075, 0.075);
	mat.set_diffuse(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	meshes["torus3"].set_material(mat);
	textures["torus3"] = texture("textures/lamecolour.jpg");
	geom_eff.add_shader("shaders/multi-light.vert", GL_VERTEX_SHADER);
	geom_eff.add_shader("shaders/multi-light.frag", GL_FRAGMENT_SHADER);
	geom_eff.build();

	// campfire
	fire = mesh(geometry_builder::create_pyramid());
	fire.get_transform().scale = vec3(2.0f / 25, 1.0f / 25, 2.0f / 25);
	fire.get_transform().translate(vec3(-5.0f, 0.52f, 1.0f));
	fire_eff.add_shader("shaders/dissolve.vert", GL_VERTEX_SHADER);
	fire_eff.add_shader("shaders/dissolve.frag", GL_FRAGMENT_SHADER);
	fire_eff.build();
	fire_tex = texture("textures/fire.jpg");
	fire_dissolve = texture("textures/blend_map4.jpg");

	// Load in necessary shaders
	eff.add_shader("shaders/terrain.vert", GL_VERTEX_SHADER);
	eff.add_shader("shaders/terrain.frag", GL_FRAGMENT_SHADER);
	eff.add_shader("shaders/part_direction.frag", GL_FRAGMENT_SHADER);
	eff.add_shader("shaders/part_weighted_texture_4.frag", GL_FRAGMENT_SHADER);
	eff.build();

	spots[0].set_position(vec3(-8.0, 5.0f, -8.0f));
	spots[0].set_range(10.0f);
	spots[0].set_light_colour(vec4(0.9f, 0.9f, 0.9f, 1.0f));
	spots[0].set_direction(normalize(vec3(1.0f, 1.0f, 1.0f)));

	// Material definitions
	light.set_ambient_intensity(vec4(0.005f, 0.005f, 0.005f, 1.0f));
	light.set_light_colour(vec4(0.9f, 0.9f, 0.9f, 1.0f));
	light.set_direction(normalize(vec3(1.0f, 1.0f, 1.0f)));
	terr.get_material().set_diffuse(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	terr.get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	terr.get_material().set_shininess(20.0f);
	terr.get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));

	// terrian trextures
	tex[0] = texture("textures/watertex.jpg");
	tex[1] = texture("textures/grass.jpg");
	tex[2] = texture("textures/stone.jpg");
	tex[3] = texture("textures/snow.jpg");

	// Set camera properties
	targetcam.set_position(vec3(-11.0f, 3.0f, -11.0f));
	targetcam.set_target(vec3(30.0f, 4.0f, 30.0f));
	targetcam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 2.414f, 1000.0f);
	freecam.set_position(vec3(pi<float>() / 6, 5.0f, 20.0f));
	freecam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 2.414f, 1000.0f);
	return true;
}

bool update(float delta_time) {
	skybox.get_transform().position = vec3(15.0f, 5.0f, 15.0f);
	// Use keyboard to move the camera - WASD
	vec3 translation(0.0f, 0.0f, 0.0f);
	if (glfwGetKey(renderer::get_window(), 'W')) {
		translation.z += 5.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'S')) {
		translation.z -= 10.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'A')) {
		translation.x -= 10.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'D')) {
		translation.x += 10.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'E')) {
		translation.y -= 10.0f * delta_time;
	}
	if (glfwGetKey(renderer::get_window(), 'R')) {
		translation.y += 10.0f * delta_time;
	}
	// target camera positions
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_1)) {
		Fcam = false;
		targetcam.set_position(vec3(8.0f, 1.75f, 8.0f));
		targetcam.set_target(vec3(-8.0, 1.5f, -8.0f));
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_2)) {
		Fcam = false;
		targetcam.set_position(vec3(5.0f, 20.0f, 10.0f));
		targetcam.set_target(vec3(60.0f, 12.0f, 30.0f));
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_3)) {
		Fcam = false;
		targetcam.set_position(vec3(-11.0f, 3.0f, -11.0f));
		targetcam.set_target(vec3(30.0f, 4.0f, 30.0f));
	}

	//fov
	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height = (quarter_pi<float>() * (static_cast<float>(renderer::get_screen_height()) /
		static_cast<float>(renderer::get_screen_width()))) /
		static_cast<float>(renderer::get_screen_height());
	double current_x;
	double current_y;
	// current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;
	// Change orientation
	delta_x = delta_x * ratio_width;
	delta_y = delta_y * ratio_height;
	// Rotate camera
	freecam.rotate(delta_x, -delta_y);
	//enable free camera
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_4)) {
		Fcam = true;
		freecam.set_target(targetcam.get_target());
		freecam.set_position(freecam.get_position());
	}
	// free camera update
	freecam.move(translation);
	//update free cam
	freecam.update(delta_time);
	cursor_x = current_x;
	cursor_y = current_y;
	// target camera update
	targetcam.update(delta_time);

	//movement
	uv_scroll += vec2(1, -delta_time * 0.025);
	uv_scroll2 += vec2(0, -delta_time * 2.5);
	skybox.get_transform().rotate(vec3(0.0f, delta_time * 0.015, 0.0f));
	theta += pi<float>() * delta_time / 2;
	meshes["orb"].get_transform().rotate(vec3(delta_time / 2, delta_time / 2, delta_time / 2));
	meshes["torus1"].get_transform().rotate(vec3(-delta_time * 2, -delta_time * 2, delta_time));
	meshes["torus2"].get_transform().rotate(vec3(0.0f, delta_time, delta_time));
	meshes["torus3"].get_transform().rotate(vec3(-delta_time / 2, -delta_time / 2, 0.0f));
	return true;
}

bool terrainrender() {
	// Bind effect
	renderer::bind(eff);
	// Create MVP matrix
	auto M = terr.get_transform().get_transform_matrix();
	auto V = targetcam.get_view();
	auto P = targetcam.get_projection();
	if (Fcam) {
		V = freecam.get_view();
		P = freecam.get_projection();
	}
	auto MVP = P * V * M;
	// Set MVP / M / N matrices
	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(terr.get_transform().get_normal_matrix()));
	glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(targetcam.get_position()));
	//Bind Terrian Material
	renderer::bind(terr.get_material(), "mat");
	//Bind Light
	renderer::bind(light, "light");
	//bind textures
	renderer::bind(tex[0], 0);
	glUniform1i(eff.get_uniform_location("tex[0]"), 0);
	renderer::bind(tex[1], 1);
	glUniform1i(eff.get_uniform_location("tex[1]"), 1);
	renderer::bind(tex[2], 2);
	glUniform1i(eff.get_uniform_location("tex[2]"), 2);
	renderer::bind(tex[3], 3);
	glUniform1i(eff.get_uniform_location("tex[3]"), 3);
	renderer::render(terr);

	return true;
}

bool skyboxrender() {
	// Render meshes
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	// Bind skybox effect
	renderer::bind(sky_eff);
	// Calculate MVP for the skybox
	auto M = skybox.get_transform().get_transform_matrix();
	auto V = targetcam.get_view();
	auto P = targetcam.get_projection();
	if (Fcam) {
		V = freecam.get_view();
		P = freecam.get_projection();
	}
	auto MVP = P * V * M;
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
	return true;
}

bool moonrender() {
	// Bind moon effect
	renderer::bind(moon_eff);
	// Create MVP matrix
	auto M = moon.get_transform().get_transform_matrix();
	auto V = targetcam.get_view();
	auto P = targetcam.get_projection();
	if (Fcam) {
		V = freecam.get_view();
		P = freecam.get_projection();
	}
	auto MVP = P * V * M;
	// Set MVP / M / N matrices
	glUniformMatrix4fv(moon_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	glUniformMatrix4fv(moon_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	glUniformMatrix3fv(moon_eff.get_uniform_location("N"), 1, GL_FALSE,
		value_ptr(moon.get_transform().get_normal_matrix()));
	// Bind material / light
	renderer::bind(moon.get_material(), "mat");
	renderer::bind(lunarlight, "light");
	// Bind / set texture
	renderer::bind(moon_tex, 0);
	glUniform1i(moon_eff.get_uniform_location("tex"), 0);
	// Bind / set normal_map
	renderer::bind(moon_norm_tex, 1);
	glUniform1i(moon_eff.get_uniform_location("normal_map"), 1);
	// Set eye position
	glUniform3fv(moon_eff.get_uniform_location("eye_pos"), 1, value_ptr(targetcam.get_position()));
	// Render moon
	renderer::render(moon);
	return true;
}

bool waterrender() {
	renderer::bind(water_eff);

	auto M = water.get_transform().get_transform_matrix();
	auto V = targetcam.get_view();
	auto P = targetcam.get_projection();
	if (Fcam) {
		V = freecam.get_view();
		P = freecam.get_projection();
	}
	auto MVP = P * V * M;
	glUniformMatrix4fv(water_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	glUniform3fv(water_eff.get_uniform_location("eye_pos"), 1, value_ptr(targetcam.get_position()));
	glUniform2fv(water_eff.get_uniform_location("UV_SCROLL"), 1, value_ptr(uv_scroll));
	renderer::render(water);
	return true;
}

bool firerender() {
	// Bind effect
	renderer::bind(fire_eff);
	auto M = fire.get_transform().get_transform_matrix();
	auto V = targetcam.get_view();
	auto P = targetcam.get_projection();
	if (Fcam) {
		V = freecam.get_view();
		P = freecam.get_projection();
	}
	auto MVP = P * V * M;

	// Set MVP matrix uniform
	glUniformMatrix4fv(fire_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	glUniform1f(fire_eff.get_uniform_location("dissolve_factor"), dissolve_factor);
	renderer::bind(fire_tex, 0);
	renderer::bind(fire_dissolve, 1);
	glUniform1i(fire_eff.get_uniform_location("tex"), 0);
	glUniform1i(fire_eff.get_uniform_location("dissolve"), 1);
	glUniform2fv(fire_eff.get_uniform_location("UV_SCROLL"), 1, value_ptr(uv_scroll2));
	// Render the mesh
	renderer::render(fire);

	return true;
}

void geometryrender() {

	for (auto &e : meshes) {
		auto m = e.second;
		// Bind effect
		renderer::bind(geom_eff);
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		//Camera type
		auto V = targetcam.get_view();
		auto P = targetcam.get_projection();
		if (Fcam) {
			V = freecam.get_view();
			P = freecam.get_projection();
		}
		auto MVP = P * V * M;
		// Set MVP / M / N matrices
		glUniformMatrix4fv(geom_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		glUniformMatrix4fv(geom_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
		glUniformMatrix3fv(geom_eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
		// Bind material
		renderer::bind(m.get_material(), "mat");
		renderer::bind(m.get_material(), "mat2");
		// Bind point / spot lights
		renderer::bind(points, "points");
		renderer::bind(spots, "spots");
		// Bind / set texture
		renderer::bind(textures[e.first], 0);
		glUniform1i(geom_eff.get_uniform_location("tex"), 0);
		// Set eye position
		glUniform3fv(geom_eff.get_uniform_location("eye_pos"), 1, value_ptr(targetcam.get_position()));
		// Render meshes
		renderer::render(m);
	}
}

bool render() {
	skyboxrender();
	moonrender();
	terrainrender();
	waterrender();
	firerender();
	geometryrender();
	return true;
}

void main() {
	// Create application
	app application("60_Terrain");
	// Set methods
	application.set_load_content(load_content);
	application.set_initialise(initialise);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}