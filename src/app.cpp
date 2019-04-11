#include "pch.h"
#include "app.h"

void App::prompt_key_bindings_color() {
	printf("[Key bindings]\n");
	printf("0: Auto\n");
	printf("1: Plane\n");
	printf("2: Sphere\n");
	printf("3: Cylinder\n");
	printf("4: Cone\n");
	printf("5: Torus\n");
	printf("SPACE: Switch screen\n");
	printf("Current mode: %s\n", type == FS_TYPE_PLANE ? "PLANE" : type == FS_TYPE_SPHERE ? "SPHERE" : type == FS_TYPE_CYLINDER ? "CYLINDER" : type == FS_TYPE_CONE ? "CONE" : type == FS_TYPE_TORUS ? "TORUS" : "ANY");
	printf("Click an object on the screen.\n");
}

void App::prompt_key_bindings_object() {
	printf("[Key bindings]\n");
	printf("Q: Rotation\n");
	printf("W: Zoom in/out\n");
	printf("HOME: Camera reset\n");
	printf("SPACE: Switch screen\n");
	printf("Current mode: %s\n", trackball_mode == Trackball::ROTATE ? "ROTATE" : "ZOOM");
}

void App::init(int width, int height) {

	fs = new FindSurface;
	type = FS_FEATURE_TYPE::FS_TYPE_PLANE;
	result.type = FS_FEATURE_TYPE::FS_TYPE_NONE;
	measurement = "Not found";

	rs.Init();
	const int max_vertex_count = rs.GetDepthWidth() * rs.GetDepthHeight();

	pc_renderer.init();
	ci_renderer.init();
	p_renderer.init();
	s_renderer.init();
	c_renderer.init();
	cn_renderer.init();
	t_renderer.init();
	f_renderer.init();
	pn_renderer.init();
	show_object = false;

	trackball.reset();
	trackball_mode = Trackball::ROTATE;

	outlier_vertices.resize(max_vertex_count);
	inlier_vertices.resize(max_vertex_count);

	trackball.home.width = static_cast<float>(width);
	trackball.home.height = static_cast<float>(height);
	trackball.reset();
}

void App::onWindowResize(GLFWwindow* window, int width, int height) {
	
	trackball.home.width = static_cast<float>(width);
	trackball.home.height = static_cast<float>(height);
	trackball.reset();

	glViewport(0, 0, width, height);
}

void App::onKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS)
		switch (key) {
		case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, 1); break;
		case GLFW_KEY_HOME:   trackball.reset(); break;
		case GLFW_KEY_Q:      if (show_object) { trackball_mode = Trackball::ROTATE; printf("Current mode: ROTATE\n"); } break;
		case GLFW_KEY_W:      if (show_object) { trackball_mode = Trackball::ZOOM; printf("Current mode: ZOOM\n"); } break;
		case GLFW_KEY_KP_0:
		case GLFW_KEY_0:      if (!show_object) { type = FS_FEATURE_TYPE::FS_TYPE_ANY; printf("Current mode: ANY\n"); } break;
		case GLFW_KEY_KP_1:
		case GLFW_KEY_1:      if (!show_object) { type = FS_FEATURE_TYPE::FS_TYPE_PLANE; printf("Current mode: PLANE\n"); } break;
		case GLFW_KEY_KP_2:
		case GLFW_KEY_2:      if (!show_object) { type = FS_FEATURE_TYPE::FS_TYPE_SPHERE; printf("Current mode: SPHERE\n"); } break;
		case GLFW_KEY_KP_3:
		case GLFW_KEY_3:      if (!show_object) { type = FS_FEATURE_TYPE::FS_TYPE_CYLINDER; printf("Current mode: CYLINDER\n"); } break;
		case GLFW_KEY_KP_4:
		case GLFW_KEY_4:      if (!show_object) { type = FS_FEATURE_TYPE::FS_TYPE_CONE; printf("Current mode: CONE\n"); } break;
		case GLFW_KEY_KP_5:
		case GLFW_KEY_5:      if (!show_object) { type = FS_FEATURE_TYPE::FS_TYPE_TORUS; printf("Current mode: TORUS\n"); } break;
		case GLFW_KEY_SPACE:  {
			show_object = !show_object;
			if (show_object) prompt_key_bindings_object();
			else prompt_key_bindings_color();
		}break;
	default: break;
	}
}

void App::onCursorPos(GLFWwindow* window, double x, double y) {
	
	if (show_object) {
		trackball.motion(static_cast<float>(x), static_cast<float>(y));
	}
}

void App::onMouseButton(GLFWwindow* window, int button, int action, int mods) {
	double _x, _y;
	glfwGetCursorPos(window, &_x, &_y);
	float x = static_cast<float>(_x), y = static_cast<float>(_y);

	if (show_object) {
		switch (action) {
		case GLFW_PRESS:   trackball.mouse(x, y, trackball_mode); break;
		case GLFW_RELEASE: trackball.mouse(x, y, Trackball::NONE); break;
		default: break;
		}
	}
	else {
		if (action == GLFW_RELEASE)
		if (!find_object(x, y)) {

		}
	}
}

void App::onScroll(GLFWwindow* window, double xoffset, double yoffset) {

}

void App::update(float time_elapsed) {

	rs.Poll(1);

	int width, height, channels;
	const unsigned char* pixels = rs.GetColorPixels(width, height, channels);
	ci_renderer.update(width, height, channels, pixels);

	trackball.update();
}

void App::render(float time_elapsed) {
	
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glm::mat4 VP = trackball.projection_matrix() * trackball.view_matrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (show_object) {
		render_object(VP);

		glViewport(0, 0, width / 5, height / 5);
		glDisable(GL_DEPTH_TEST);
		{
			render_colorimage();
		}
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, width, height);

		pn_renderer.render(5, 5, 170, 100, 0, 0, 0, 0.3f);
		f_renderer.render(10, 20, 1, 1, 1, "[Key bindings]");
		f_renderer.render(10, 40, 1, 1, 1, trackball_mode==Trackball::ROTATE ? "Q: Rotation [*]" : "Q: Rotation");
		f_renderer.render(10, 60, 1, 1, 1, trackball_mode==Trackball::ZOOM ? "W: Zoom in/out [*]" : "W: Zoom in/out");
		f_renderer.render(10, 80, 1, 1, 1, "HOME: Camera reset");
		f_renderer.render(10, 100, 1, 1, 1, "SPACE: Switch screen");
	}
	else {
		render_colorimage();

		glEnable(GL_SCISSOR_TEST);
		glScissor(0, 0, width / 5, height / 5);
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, width / 5, height / 5);
			render_object(VP);
			glViewport(0, 0, width, height);
		}
		glScissor(0, 0, width, height);
		glDisable(GL_SCISSOR_TEST);

		// text rendering
		pn_renderer.render(5, 5, 170, 160, 0, 0, 0, 0.3f);
		f_renderer.render(10, 20, 1, 1, 1, "[Key bindings]");
		f_renderer.render(10, 40, 1, 1, 1, type==FS_TYPE_ANY ? "0: Auto [*]" : "0: Auto");
		f_renderer.render(10, 60, 1, 1, 1, type==FS_TYPE_PLANE ? "1: Plane [*]" : "1: Plane");
		f_renderer.render(10, 80, 1, 1, 1, type==FS_TYPE_SPHERE ? "2: Sphere [*]" : "2: Sphere");
		f_renderer.render(10, 100, 1, 1, 1, type==FS_TYPE_CYLINDER ? "3: Cylinder [*]" : "3: Cylinder");
		f_renderer.render(10, 120, 1, 1, 1, type==FS_TYPE_CONE ? "4: Cone [*]" : "4: Cone");
		f_renderer.render(10, 140, 1, 1, 1, type==FS_TYPE_TORUS ? "5: Torus [*]" : "5: Torus");
		f_renderer.render(10, 160, 1, 1, 1, "SPACE: Switch screen");

		pn_renderer.render(width - 240, 5, 220, 20, 0, 0, 0, 0.3f);
		f_renderer.render(width - 240, 20, 1, 1, 1, "Click an object on the screen.");
	}

	int length = measurement.length() * 8;
	pn_renderer.render(width - length - 25, height - 30, length + 10, 20, 0, 0, 0, 0.3f);
	f_renderer.render(width - length - 20, height - 15, 1, 1, 1, measurement.c_str());
}

void App::cleanup() {
	
	delete fs;

	rs.Release();

	pc_renderer.release();
	ci_renderer.release();
	p_renderer.release();
	s_renderer.release();
	c_renderer.release();
	cn_renderer.release();
	t_renderer.release();
	f_renderer.release();
	pn_renderer.release();
}

void App::render_colorimage() {

	ci_renderer.render();
}

void App::render_object(const glm::mat4& VP) {

	switch (result.type) {
	case FS_TYPE_PLANE:    p_renderer.render(VP, glm::vec4(1, 0, 0, 1)); break;
	case FS_TYPE_SPHERE:   s_renderer.render(VP, glm::vec4(1, 1, 0, 1)); break;
	case FS_TYPE_CYLINDER: c_renderer.render(VP, glm::vec4(0, 1, 0, 1), 3); break;
	case FS_TYPE_CONE:     cn_renderer.render(VP, glm::vec4(0, 1, 1, 1), 2); break;
	case FS_TYPE_TORUS:    t_renderer.render(VP, glm::vec4(0, 0, 1, 1)); break;
	default: break;
	}

	if (result.type != FS_TYPE_NONE) pc_renderer.render_arrays(VP);//pc_renderer.render_elements(VP, ci_renderer.texture);
}

bool App::find_object(float x, float y) {
	
	int window_width, window_height;
	glfwGetWindowSize(window, &window_width, &window_height);

	// fetch pointcloud
	rs.GetDepthPointsWithColors(1, outlier_count, outlier_vertices.data());
	
	// ray casting
	glm::vec2 tc = glm::clamp(glm::vec2(x / window_width, y / window_height), 0.f, 1.f);
	glm::vec2 color_resolution(rs.GetColorResolution());
	glm::vec3 p0 = rs.TransformColorToDepth(rs.DeprojectColor(tc * color_resolution, 0.f));
	glm::vec3 p1 = rs.TransformColorToDepth(rs.DeprojectColor(tc * color_resolution, 1.f));

	ray r{ p0, glm::normalize(p1 - p0) };
	int index = cast_ray(r);

	if (index == -1) return false;

	// estimate parameters
	float depth = outlier_vertices[index].pos.z;
	float average_error = glm::max(depth*0.026119565f - 0.00712f, 0.002f); // this formula comes from heuristic estimation and not accurate but does no need to be.
	float height = depth * glm::tan(glm::radians(rs.GetColorVFov() * 0.5f));
	float scale = 2.f * height / static_cast<float>(rs.GetColorHeight());
	float density = scale * 2.7f;
	float touch_radius = 50.f * scale; // 50 px radius

	// run FindSurface
	fs->set_type(type);
	fs->set_pointcloud(reinterpret_cast<const float*>(outlier_vertices.data()), outlier_count, sizeof(vertex));
	fs->set_accuracy(average_error);
	fs->set_mean_distance(density);
	fs->set_touch_radius(touch_radius);

	result = fs->find_surface(index, 50.f);

	if (result.type == FS_FEATURE_TYPE::FS_TYPE_NONE) {
		measurement = "Not found";
		printf("%s\n", measurement.c_str());
		return false;
	}
	
	// get indices of inliers
	std::vector<unsigned char> outlier_flags = fs->get_outlier_flags();
	vertex* outliers = outlier_vertices.data();
	vertex* inliers = inlier_vertices.data();
	inlier_count = 0;
	for (unsigned char is_outlier : outlier_flags) {
		if (!is_outlier) {
			*inliers++ = *outliers;
			inlier_count++;
		}
		++outliers;
	}
 	pc_renderer.update_vertices(inlier_count, inlier_vertices.data(), GL_STATIC_DRAW);
	pc_renderer.update_texture(ci_renderer.texture);

	glm::vec3 cam_center;
	float cam_radius;
	switch (result.type) {
	case FS_FEATURE_TYPE::FS_TYPE_PLANE: {
		p_info = result;

		float hori = glm::length(p_info.get_major_edge_vector())*100.f;
		float vert = glm::length(p_info.get_minor_edge_vector())*100.f;
		char buf[256]; snprintf(buf, 256, "Plane: %.2f cm x %.2f cm", hori, vert);
		measurement = buf;

		cam_center = p_info.get_center();
		cam_radius = glm::distance(glm::vec3(p_info.ll), cam_center);
		p_renderer.update(p_info.data());
		break;
	}
	case FS_FEATURE_TYPE::FS_TYPE_SPHERE: {
		s_info = result;

		float radius = s_info.radius * 100.f;
		char buf[256]; snprintf(buf, 256, "Sphere: r = %.2f cm", radius);
		measurement = buf;

		cam_center = s_info.center;
		cam_radius = s_info.radius;
		s_renderer.update(s_info.data());
		break;
	}
	case FS_FEATURE_TYPE::FS_TYPE_CYLINDER: {
		c_info = result;

		float radius = c_info.top_radius * 100.f;
		float length = c_info.get_length() * 100.f;
		char buf[256]; snprintf(buf, 256, "Cylinder: l = %.2f cm, r = %.2f cm", length, radius);
		measurement = buf;

		cam_center = c_info.get_center();
		cam_radius = glm::length(glm::vec2(c_info.get_length(), c_info.top_radius));
		c_renderer.update(c_info.data());
		break;
	}
	case FS_FEATURE_TYPE::FS_TYPE_CONE: {
		cn_info = result;

		float top_radius = cn_info.top_radius * 100.f;
		float bottom_radius = cn_info.bottom_radius * 100.f;
		float length = cn_info.get_length() * 100.f;
		char buf[256]; snprintf(buf, 256, "Cone: l = %.2f cm, tr = %.2f cm, br = %.2f cm", length, top_radius, bottom_radius);
		measurement = buf;

		cam_center = cn_info.get_center();
		cam_radius = glm::length(glm::vec2(cn_info.get_length(), cn_info.bottom_radius));
		cn_renderer.update(cn_info.data());
		break;
	}
	case FS_FEATURE_TYPE::FS_TYPE_TORUS: {
		const int inlier_count = fs->get_inlier_count();
		std::vector<glm::vec3> inlier_points(inlier_count);
		fs->get_inliers(inlier_count * sizeof(glm::vec3), inlier_points.data());
		t_info = torus_info(result, inlier_points);

		float mean_radius = t_info.mean_radius * 100.f;
		float tube_radius = t_info.tube_radius * 100.f;
		int tube_angle = static_cast<int>(glm::degrees(t_info.tube_angle * 2.f));
		char buf[256]; snprintf(buf, 256, "Torus: mr = %.2f cm, tr = %.2f cm, angle = %d deg", mean_radius, tube_radius, tube_angle);
		measurement = buf;

		cam_center = t_info.center;
		cam_radius = t_info.mean_radius + t_info.tube_radius;
		t_renderer.update(t_info.data());
		break;
	}
	default: break;
	}

	trackball.home.at = cam_center;
	trackball.home.eye = cam_center - glm::vec3(0, 0, cam_radius * 1.1f);
	trackball.home.up = glm::vec3(0, -1, 0);
	trackball.home.dnear = cam_radius * 0.1f;
	trackball.home.dfar = cam_radius * 2.2f;
	trackball.home.zoom_factor = trackball.home.height / (cam_radius * 3);
	trackball.reset();

	printf("%s\n", measurement.c_str());

	return true;
}

int App::cast_ray(const ray& r) {

	if (outlier_count == 0) return -1;

	int min_index = -1;
	float min_dist = std::numeric_limits<float>::max();
	const vertex* point = outlier_vertices.data();

	for (int k = 0; k < outlier_count; k++) {
		float dist = r.get_distance(point++->pos);
		if (dist < min_dist) {
			min_index = k;
			min_dist = dist;
		}
	}

	return min_index;
}