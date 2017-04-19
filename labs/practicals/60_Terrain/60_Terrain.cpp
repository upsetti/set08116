#include <glm/glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

mesh terr;
effect eff;
mesh skybox;
effect sky_eff;
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
free_camera cam;
cubemap cube_map;
directional_light light;
texture tex[4];
float theta = 0.0f;
//flames
mesh fire;
texture fire_tex;
effect fire_eff;
texture fire_dissolve;
vec2 uv_scroll2;
float dissolve_factor = 1.0f;
//filter
frame_buffer frame;
geometry screenquad;
bool filter = false;
effect filter_eff;
//geometry
map<string, mesh> meshes;
map<string, texture> textures;
vector<spot_light> flames(5);


void generate_terrain(geometry &geom, const texture &height_map, unsigned int width, unsigned int depth,
	float height_scale) {
	// Contains our position data
	vector<vec3> positions;
	// Contains our normal data
	vector<vec3> normals;
	// Contains our texture coordinate data
	vector<vec2> tex_coords;
	// Contains our texture weights
	vector<vec4> tex_weights;
	// Contains our index data
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

	// Part 1 - Iterate through each point, calculate vertex and add to vector
	for (int x = 0; x < height_map.get_width(); ++x) {
		// Calculate x position of point
		point.x = -(width / 2.0f) + (width_point * static_cast<float>(x));

		for (int z = 0; z < height_map.get_height(); ++z) {
			// *********************************
			// Calculate z position of point
			point.z = -(depth / 2.0f) + (depth_point * static_cast<float>(z));
			// *********************************
			// Y position based on red component of height map data
			point.y = data[(z * height_map.get_width()) + x].y * height_scale;
			// Add point to position data
			positions.push_back(point);
		}
	}

	// Part 1 - Add index data
	for (unsigned int x = 0; x < height_map.get_width() - 1; ++x) {
		for (unsigned int y = 0; y < height_map.get_height() - 1; ++y) {
			// Get four corners of patch
			unsigned int top_left = (y * height_map.get_width()) + x;
			unsigned int top_right = (y * height_map.get_width()) + x + 1;
			// *********************************
			unsigned int bottom_left = ((y + 1) * height_map.get_width()) + x;
			unsigned int bottom_right = ((y + 1) * height_map.get_height()) + x + 1;
			// *********************************
			// Push back indices for triangle 1 (tl,br,bl)
			indices.push_back(top_left);
			indices.push_back(bottom_right);
			indices.push_back(bottom_left);
			// Push back indices for triangle 2 (tl,tr,br)
			// *********************************
			indices.push_back(top_left);
			indices.push_back(top_right);
			indices.push_back(bottom_right);
			// *********************************
		}
	}

	// Resize the normals buffer
	normals.resize(positions.size());

	// Part 2 - Calculate normals for the height map
	for (unsigned int i = 0; i < indices.size() / 3; ++i) {
		// Get indices for the triangle
		auto idx1 = indices[i * 3];
		auto idx2 = indices[i * 3 + 1];
		auto idx3 = indices[i * 3 + 2];

		// Calculate two sides of the triangle
		vec3 side1 = positions[idx1] - positions[idx3];
		vec3 side2 = positions[idx1] - positions[idx2];

		// Normal is normal(cross product) of these two sides
		// *********************************
		vec3 n = cross(side2, side1);

		// Add to normals in the normal buffer using the indices for the triangle
		normals[idx1] = normals[idx1] + n;
		normals[idx2] = normals[idx2] + n;
		normals[idx3] = normals[idx3] + n;
		// *********************************
	}

	// Normalize all the normals
	for (auto &n : normals) {
		// *********************************
		n = normalize(n);
		// *********************************
	}

	// Part 3 - Add texture coordinates for geometry
	for (unsigned int x = 0; x < height_map.get_width(); ++x) {
		for (unsigned int z = 0; z < height_map.get_height(); ++z) {
			tex_coords.push_back(vec2(width_point * x, depth_point * z));
		}
	}

	// Part 4 - Calculate texture weights for each vertex
	for (unsigned int x = 0; x < height_map.get_width(); ++x) {
		for (unsigned int z = 0; z < height_map.get_height(); ++z) {
			// Calculate terraintex weight
			vec4 tex_weight(clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.0f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.15f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.5f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.9f) / 0.25f, 0.0f, 1.0f));

			// *********************************
			// Sum the components of the vector
			float total = tex_weight.x + tex_weight.y + tex_weight.z + tex_weight.a;
			// Divide weight by sum
			tex_weight = tex_weight / total;
			// Add tex weight to weights
			tex_weights.push_back(tex_weight);
			// *********************************
		}
	}

	// Add necessary buffers to the geometry
	geom.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	geom.add_buffer(normals, BUFFER_INDEXES::NORMAL_BUFFER);
	geom.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	geom.add_buffer(tex_weights, BUFFER_INDEXES::TEXTURE_COORDS_1);
	geom.add_index_buffer(indices);

	// Delete data
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

	// Geometry to load into
	geometry geom;

	// Load height map
	texture height_map("textures/hmap.png");

	// Generate terrain
	generate_terrain(geom, height_map, 20, 20, 2.0f);

	// Use geometry to create terrain mesh
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
	
	//campfire
	fire = mesh(geometry_builder::create_pyramid());
	fire.get_transform().scale = vec3(2.0f/50, 1.0f/50, 2.0f/50);
	fire.get_transform().translate(vec3(-5.0f, 0.52f, 1.0f));
	fire_eff.add_shader("shaders/dissolve.vert", GL_VERTEX_SHADER);
	fire_eff.add_shader("shaders/dissolve.frag", GL_FRAGMENT_SHADER);
	fire_eff.build();
	fire_tex = texture("textures/fire.jpg");
	fire_dissolve = texture("textures/blend_map4.jpg");
	flames[0].set_position(vec3(-5.0f, 0.52f, 1.0f));
	flames[0].set_range(15.0f);
	flames[0].set_direction(normalize(vec3(0.0f, -1.0f, 0.0f)));
	flames[0].set_light_colour(vec4(2.28f, 0.8f, 0.34f, 1.0f));

	//filter
	// Create frame buffer - use screen width and height
	frame = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	vector<vec3> positions{ vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f),
		vec3(1.0f, 1.0f, 0.0f) };
	vector<vec2> tex_coords{ vec2(0.0, 0.0), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f) };
	screenquad.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	screenquad.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	screenquad.set_type(GL_TRIANGLE_STRIP);
	filter_eff.add_shader("shaders/simple_texture.vert", GL_VERTEX_SHADER);
	filter_eff.add_shader("shaders.greyscale.frag", GL_FRAGMENT_SHADER);
	filter_eff.build();


	// Load in necessary shaders
	eff.add_shader("60_Terrain/terrain.vert", GL_VERTEX_SHADER);
	eff.add_shader("60_Terrain/terrain.frag", GL_FRAGMENT_SHADER);
	eff.add_shader("shaders/part_direction.frag", GL_FRAGMENT_SHADER);
	eff.add_shader("60_Terrain/part_weighted_texture_4.frag", GL_FRAGMENT_SHADER);
	eff.build();

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
	cam.set_position(vec3(0.0f, 5.0f, 10.0f));
	cam.set_target(vec3(0.0f, 0.0f, 0.0f));
	cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	return true;
}

bool update(float delta_time) {
	skybox.get_transform().position = vec3(15.0f, 5.0f, 15.0f);
	// The ratio of pixels to rotation - remember the fov
	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height =
		(quarter_pi<float>() * renderer::get_screen_aspect()) / static_cast<float>(renderer::get_screen_height());
	static double cursor_x = 0.0;
	static double cursor_y = 0.0;
	double current_x;
	double current_y;
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;
	// Multiply deltas by ratios - gets actual change in orientation
	delta_x *= ratio_width;
	delta_y *= ratio_height;
	// Rotate cameras by delta
	cam.rotate(delta_x, -delta_y);
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

	//filter
	if (glfwGetKey(renderer::get_window(), 'F')) {
		if (filter = false) {
			filter = true;
		}
		else if (filter = true) {
			filter = false;
		}
	}

	uv_scroll += vec2(1, -delta_time * 0.025);
	uv_scroll2 += vec2(0, -delta_time * 2.5);
	skybox.get_transform().rotate(vec3(0.0f, delta_time * 0.015, 0.0f));
	theta += pi<float>() * delta_time / 2;
	/*meshes["orb"].get_transform().rotate(vec3(delta_time / 2, delta_time / 2, delta_time / 2));
	meshes["torus1"].get_transform().rotate(vec3(-delta_time * 2, -delta_time * 2, delta_time));
	meshes["torus2"].get_transform().rotate(vec3(0.0f, delta_time, delta_time));
	meshes["torus3"].get_transform().rotate(vec3(-delta_time / 2, -delta_time / 2, 0.0f));*/
	// Move camera
	cam.move(translation);
	// Update the camera
	cam.update(delta_time);
	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;
	return true;
}

bool terrainrender() {
	// Bind effect
	renderer::bind(eff);
	// Create MVP matrix
	auto M = terr.get_transform().get_transform_matrix();
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set N matrix uniform
	glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(terr.get_transform().get_normal_matrix()));
	// *********************************
	// Set eye_pos uniform to camera position
	glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
	// *********************************
	//Bind Terrian Material
	renderer::bind(terr.get_material(), "mat");
	// Bind Light
	renderer::bind(light, "light");
	// Bind Tex[0] to TU 0, set uniform
	renderer::bind(tex[0], 0);
	glUniform1i(eff.get_uniform_location("tex[0]"), 0);
	// *********************************
	//Bind Tex[1] to TU 1, set uniform
	renderer::bind(tex[1], 1);
	glUniform1i(eff.get_uniform_location("tex[1]"), 1);
	// Bind Tex[2] to TU 2, set uniform
	renderer::bind(tex[2], 2);
	glUniform1i(eff.get_uniform_location("tex[2]"), 2);
	// Bind Tex[3] to TU 3, set uniform
	renderer::bind(tex[3], 3);
	glUniform1i(eff.get_uniform_location("tex[3]"), 3);
	// *********************************
	// Render terrain
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
	auto V = cam.get_view();
	auto P = cam.get_projection();
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
		auto V = cam.get_view();
		auto P = cam.get_projection();
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
		glUniform3fv(moon_eff.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
		// Render moon
		renderer::render(moon);
		return true;
}

bool waterrender() {
	renderer::bind(water_eff);

	auto M = water.get_transform().get_transform_matrix();
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V * M;
	glUniformMatrix4fv(water_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	glUniform3fv(water_eff.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
	glUniform2fv(water_eff.get_uniform_location("UV_SCROLL"), 1, value_ptr(uv_scroll));
	renderer::render(water);
	return true;
}

bool firerender() {
	// Bind effect
	renderer::bind(fire_eff);
	auto M = fire.get_transform().get_transform_matrix();
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V * M;

	// Set MVP matrix uniform
	glUniformMatrix4fv(fire_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	glUniform1f(fire_eff.get_uniform_location("dissolve_factor"), dissolve_factor);
	renderer::bind(fire_tex, 0);
	renderer::bind(fire_dissolve, 1);
	//renderer::bind(flames, "spots");
	glUniform1i(fire_eff.get_uniform_location("tex"), 0);
	glUniform1i(fire_eff.get_uniform_location("dissolve"), 1);
	glUniform2fv(fire_eff.get_uniform_location("UV_SCROLL"), 1, value_ptr(uv_scroll2));
	// Render the mesh
	renderer::render(fire);

	return true;
}

bool filterrender() {
	renderer::set_render_target(frame);
	// Clear frame
	renderer::clear();
	// Set render target back to the screen
	renderer::set_render_target();
	// Bind Tex effect
	renderer::bind(filter_eff);
	// MVP is now the identity matrix
	auto MVP = mat4(1);
	// Set MVP matrix uniform
	glUniformMatrix4fv(filter_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Bind texture from frame buffer
	renderer::bind(frame.get_frame(), 1);
	// Set the tex uniform
	glUniform1i(filter_eff.get_uniform_location("tex"), 1);
	// Render the screen quad
	renderer::render(screenquad);
	// *********************************
	return true;
}

bool render() {
	skyboxrender();
	moonrender();
	terrainrender();
	waterrender();
	firerender();
	filterrender();
	return true;
}

void main() {
	// Create application
	app application("60_Terrain");
	// Set methods
	application.set_load_content(load_content);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}