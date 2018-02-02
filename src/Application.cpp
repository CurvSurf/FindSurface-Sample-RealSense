#include "Application.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "realsense.lib")
#pragma comment(lib, "FindSurface.lib")

bool Application::init() {
	if (init_RealSense() == false) return false;
	if (init_FindSurface() == false) return false;

	init_data();

	glfwInit();
	window = glfwCreateWindow(width, height, title, nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int b, int a, int m) {reinterpret_cast<Application*>(glfwGetWindowUserPointer(w))->on_mouse_button(w, b, a, m); });
	glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {reinterpret_cast<Application*>(glfwGetWindowUserPointer(w))->on_cursor_pos(w, x, y); });
	glfwSetKeyCallback(window, [](GLFWwindow* w, int k, int s, int a, int m) {reinterpret_cast<Application*>(glfwGetWindowUserPointer(w))->on_key(w, k, s, a, m); });
	glfwSetScrollCallback(window, [](GLFWwindow* w, double x, double y) {reinterpret_cast<Application*>(glfwGetWindowUserPointer(w))->on_wheel(w, x, y); });
		
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "GLEW: failed to initialize GLEW.\n");
		return false;
	}

	init_OpenGL();

	return true;
}

bool Application::init_RealSense() {
	rs::log_to_console(rs::log_severity::warn);

	if (ctx.get_device_count() == 0) {
		fprintf(stderr, "RealSense: there is no RealSense device connected.\n");
		return false;
	}

	dev = ctx.get_device(0);
	dev->enable_stream(rs::stream::depth, rs::preset::best_quality);
	dev->enable_stream(rs::stream::color, rs::preset::best_quality);
	dev->start();

	depth_intrin = dev->get_stream_intrinsics(rs::stream::depth);
	depth_to_color = dev->get_extrinsics(rs::stream::depth, rs::stream::rectified_color);
	color_to_depth = dev->get_extrinsics(rs::stream::rectified_color, rs::stream::depth);
	color_intrin = dev->get_stream_intrinsics(rs::stream::rectified_color);
	scale = dev->get_depth_scale();

	return true;
}

void Application::release_RealSense() {
	dev->stop();
}

bool Application::init_FindSurface() {
	switch (createFindSurface(&fs)) {
	case FS_OUT_OF_MEMORY:		fprintf(stderr, "FindSurface: failed to create a context (out of memory).\n"); return false;
	case FS_LICENSE_EXPIRED:	fprintf(stderr, "FindSurface: failed to create a context (license expired).\n"); return false;
	case FS_LICENSE_UNKNOWN:	fprintf(stderr, "FindSurface: failed to create a context (license unknown).\n"); return false;
	}

	// initialize parameters (not mandatory, but recommended)
	setFindSurfaceParamFloat(fs, FS_PARAMS::FS_PARAM_ACCURACY, 0.003f);
	setFindSurfaceParamFloat(fs, FS_PARAMS::FS_PARAM_MEAN_DIST, 0.01f);
	setFindSurfaceParamFloat(fs, FS_PARAMS::FS_PARAM_TOUCH_R, 0.045f);

	return true;
}

void Application::release_FindSurface() {
	cleanUpFindSurface(fs);
	releaseFindSurface(fs);
}

void Application::init_data() {
	size_t capacity = depth_intrin.width*depth_intrin.height;
	depth_points.reserve(capacity);
	depth_colors.reserve(capacity);

	inlier_points.reserve(capacity);
	inlier_colors.reserve(capacity);
}

void Application::init_OpenGL() {

	ShaderSource::init();

	// vertex data array
	std::vector<smath::float3> sphere_vertex_data;
	std::vector<unsigned int> sphere_index_data;
	sgeometry::CreateSphereVertexData(sphere_vertex_data, sphere_index_data, 3);

	std::vector<smath::float3> cylinder_vertex_data;
	std::vector<unsigned int> cylinder_index_data;
	sgeometry::CreateCylinderVertexData(cylinder_vertex_data, cylinder_index_data);

	std::vector<smath::float3> torus_vertex_data;
	std::vector<unsigned int> torus_index_data;
	sgeometry::CreateTorusVertexData(torus_vertex_data, torus_index_data);

	std::vector<smath::float3> geometry_vertex_data;
	geometry_vertex_data.reserve(sphere_vertex_data.size() + cylinder_vertex_data.size() + torus_vertex_data.size());
	geometry_vertex_data.insert(geometry_vertex_data.end(), sphere_vertex_data.cbegin(), sphere_vertex_data.cend());
	geometry_vertex_data.insert(geometry_vertex_data.end(), cylinder_vertex_data.cbegin(), cylinder_vertex_data.cend());
	geometry_vertex_data.insert(geometry_vertex_data.end(), torus_vertex_data.cbegin(), torus_vertex_data.cend());

	std::vector<unsigned int> geometry_index_data;
	geometry_index_data.reserve(sphere_index_data.size() + cylinder_index_data.size() + torus_index_data.size());
	geometry_index_data.insert(geometry_index_data.end(), sphere_index_data.cbegin(), sphere_index_data.cend());
	geometry_index_data.insert(geometry_index_data.end(), cylinder_index_data.cbegin(), cylinder_index_data.cend());
	geometry_index_data.insert(geometry_index_data.end(), torus_index_data.cbegin(), torus_index_data.cend());
	
	// vao, vbo, ibo for geometries
	sgl::VertexArray geometry_vao;
	geometry_vao.Init();
	geometry_vao.Bind();
	
	sgl::VertexBuffer geometry_vbo;
	geometry_vbo.Init();
	geometry_vbo.Data(geometry_vertex_data.size(), sizeof(smath::float3), geometry_vertex_data.data(), GL_STATIC_DRAW);
	geometry_vbo.Bind();
	geometry_vao.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	sgl::IndexBuffer geometry_ibo;
	geometry_ibo.Init();
	geometry_ibo.Data(geometry_index_data.size(), sizeof(unsigned int), geometry_index_data.data(), GL_STATIC_DRAW);
	geometry_ibo.Bind();

	GLsizei sphere_vertex_count = GLsizei(sphere_vertex_data.size());
	GLsizei sphere_index_count = GLsizei(sphere_index_data.size());
	GLsizei cylinder_vertex_count = GLsizei(cylinder_vertex_data.size());
	GLsizei cylinder_index_count = GLsizei(cylinder_index_data.size());
	GLsizei torus_vertex_count = GLsizei(torus_vertex_data.size());
	GLsizei torus_index_count = GLsizei(torus_index_data.size());

	// renderer
	plane_renderer.program.Init(ShaderSource::vs_src["plane"], ShaderSource::fs_src["plane"]);
	plane_renderer.vertex_array = geometry_vao;
	plane_renderer.position_buffer = geometry_vbo;
	plane_renderer.index_buffer = geometry_ibo;
	plane_renderer.draw = sgl::DrawArrays{ GL_TRIANGLES, 0, 6 };

	sphere_renderer.program.Init(ShaderSource::vs_src["sphere"], ShaderSource::fs_src["sphere"]);
	sphere_renderer.vertex_array = geometry_vao;
	sphere_renderer.position_buffer = geometry_vbo;
	sphere_renderer.index_buffer = geometry_ibo;
	sphere_renderer.draw = sgl::DrawElementsBaseVertex{ GL_TRIANGLES, sphere_index_count, GL_UNSIGNED_INT, 0, 0 };

	cylinder_renderer.program.Init(ShaderSource::vs_src["cylinder"], ShaderSource::fs_src["cylinder"]);
	cylinder_renderer.vertex_array = geometry_vao;
	cylinder_renderer.position_buffer = geometry_vbo;
	cylinder_renderer.index_buffer = geometry_ibo;
	cylinder_renderer.draw = sgl::DrawElementsBaseVertex{ GL_TRIANGLES, cylinder_index_count, GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(sphere_index_count * sizeof(unsigned int)), sphere_vertex_count };

	cone_renderer.program.Init(ShaderSource::vs_src["cone"], ShaderSource::fs_src["cone"]);
	cone_renderer.vertex_array = geometry_vao;
	cone_renderer.position_buffer = geometry_vbo;
	cone_renderer.index_buffer = geometry_ibo;
	cone_renderer.draw = cylinder_renderer.draw;

	torus_renderer.program.Init(ShaderSource::vs_src["torus"], ShaderSource::fs_src["torus"]);
	torus_renderer.vertex_array = geometry_vao;
	torus_renderer.position_buffer = geometry_vbo;
	torus_renderer.index_buffer = geometry_ibo;
	torus_renderer.draw = sgl::DrawElementsBaseVertex{ GL_TRIANGLES, torus_index_count, GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>((sphere_index_count + cylinder_index_count) * sizeof(unsigned int)), sphere_vertex_count + cylinder_vertex_count };

	depth_renderer.program.Init(ShaderSource::vs_src["point_cloud"], ShaderSource::fs_src["point_cloud"]);
	depth_renderer.vertex_array.Init();
	depth_renderer.vertex_array.Bind();
	depth_renderer.position_buffer.Init();
	depth_renderer.position_buffer.Bind();
	depth_renderer.vertex_array.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	depth_renderer.color_buffer.Init();
	depth_renderer.color_buffer.Bind();
	depth_renderer.vertex_array.AttribIPointer(1, 3, GL_UNSIGNED_BYTE, 0, 0);
	depth_renderer.draw = sgl::DrawArrays{ GL_POINTS, 0, 0/*using position_buffer.count instead*/ };

	inlier_renderer.program = depth_renderer.program;
	inlier_renderer.vertex_array.Init();
	inlier_renderer.vertex_array.Bind();
	inlier_renderer.position_buffer.Init();
	inlier_renderer.position_buffer.Bind();
	inlier_renderer.vertex_array.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	inlier_renderer.color_buffer.Init();
	inlier_renderer.color_buffer.Bind();
	inlier_renderer.vertex_array.AttribIPointer(1, 3, GL_UNSIGNED_BYTE, 0, 0);
	inlier_renderer.draw = sgl::DrawArrays{ GL_POINTS, 0, 0/*using position_buffer.count instead*/ };
	
	const GLsizeiptr buffer_size = color_intrin.width*color_intrin.height * 3;
	image_renderer.program.Init(ShaderSource::vs_src["color"], ShaderSource::fs_src["color"]);
	glGenTextures(1, &image_renderer.texture);
	glBindTexture(GL_TEXTURE_2D, image_renderer.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, color_intrin.width, color_intrin.height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	image_renderer.PBO[0].Init(GL_PIXEL_UNPACK_BUFFER);
	image_renderer.PBO[0].Data(buffer_size, nullptr, GL_STREAM_DRAW);
	image_renderer.PBO[1].Init(GL_PIXEL_UNPACK_BUFFER);
	image_renderer.PBO[1].Data(buffer_size, nullptr, GL_STREAM_DRAW);

	trackball.curr.eye = smath::float3{};
	trackball.curr.at = smath::float3{ 0, 0, 1 };
	trackball.curr.up = smath::float3{ 0, -1, 0 };
	trackball.curr.width = float(width) / 1000;
	trackball.curr.height = float(height) / 1000;
	trackball.curr.dnear = 0.001f;
	trackball.curr.dfar = 20.0f;
	trackball.prev = trackball.home = trackball.curr;

	trackball2.curr.eye = smath::float3{};
	trackball2.curr.at = smath::float3{ 0, 0, 1 };
	trackball2.curr.up = smath::float3{ 0, -1, 0 };
	trackball2.curr.width = float(width) / 1000;
	trackball2.curr.height = float(height) / 1000;
	trackball2.curr.dnear = 0.001f;
	trackball2.curr.dfar = 20.0f;
	trackball2.prev = trackball2.home = trackball2.curr;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
}

void Application::release_OpenGL() {

	plane_renderer.program.Release();
	plane_renderer.vertex_array.Release();
	plane_renderer.position_buffer.Release();
	plane_renderer.index_buffer.Release();
	
	sphere_renderer.program.Release();
	sphere_renderer.vertex_array.Release();
	sphere_renderer.position_buffer.Release();
	sphere_renderer.index_buffer.Release();

	cylinder_renderer.program.Release();
	cylinder_renderer.vertex_array.Release();
	cylinder_renderer.position_buffer.Release();
	cylinder_renderer.index_buffer.Release();

	cone_renderer.program.Release();
	cone_renderer.vertex_array.Release();
	cone_renderer.position_buffer.Release();
	cone_renderer.index_buffer.Release();

	torus_renderer.program.Release();
	torus_renderer.vertex_array.Release();
	torus_renderer.position_buffer.Release();
	torus_renderer.index_buffer.Release();

	depth_renderer.program.Release();
	depth_renderer.vertex_array.Release();
	depth_renderer.position_buffer.Release();
	depth_renderer.color_buffer.Release();
	
	inlier_renderer.program.Release();
	inlier_renderer.vertex_array.Release();
	inlier_renderer.position_buffer.Release();
	inlier_renderer.color_buffer.Release();

	image_renderer.program.Release();
	glDeleteTextures(1, &image_renderer.texture);
	image_renderer.PBO[0].Release();
	image_renderer.PBO[1].Release();
}

void Application::run() {
	int frame = 0;
	double t0 = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		dev->wait_for_frames();

		double t1 = glfwGetTime();
		double dt = t1 - t0;
		t0 = t1;
		frame++;

		update(frame, dt);
		render(frame, dt);

		ms_log[t_index] = dt;
		double avg_ms = std::accumulate(ms_log, ms_log + 60, 0.0) / 60;
		t_index = (t_index + 1) % 60;
		glfwSetWindowTitle(window, (std::string(title) + "(" + std::to_string(1.0 / avg_ms) + " fps, " + std::to_string(avg_ms) + " ms)").c_str());
		glfwSwapBuffers(window);
	}

	finalize();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::update(int frame, double time_elapsed) {
	depth_points.clear();
	depth_colors.clear();

	// 1. fetch point clouds from the Intel RealSense device.
	const uint16_t* depth_image = (const uint16_t*)dev->get_frame_data(rs::stream::depth);
	color_image = (uint8_t*)dev->get_frame_data(rs::stream::rectified_color);

	// 2. We have to filter out *dead* pixels that do not have depth values due to measurement errors,
	//	  such as obsorbing IR of black surfaces or too much far distant surfaces.
	int depth_width = depth_intrin.width;
	int depth_height = depth_intrin.height;
	for (int dy = 0; dy < depth_height; dy++) {
		for (int dx = 0; dx < depth_width; dx++) {
			uint16_t depth_value = depth_image[dy*depth_width + dx];
			float depth_in_meters = depth_value * scale;

			if (depth_value == 0) continue; // zero depth means it is one of the dead pixels.

			rs::float2 depth_pixel = { float(dx), float(dy) };
			rs::float3 depth_point = depth_intrin.deproject(depth_pixel, depth_in_meters);
			rs::float3 color_point = depth_to_color.transform(depth_point);
			rs::float2 color_pixel = color_intrin.project(color_point);

			ubyte3 depth_color = {};
			const int cx = int(std::round(color_pixel.x)), cy = int(std::round(color_pixel.y));
			if (cx >= 0 && cx < color_intrin.width && cy >= 0 && cy < color_intrin.height) {
				depth_color = *((ubyte3*)(color_image + 3 * (cy*color_intrin.width + cx)));
			}

			depth_points.push_back(depth_point);
			depth_colors.push_back(depth_color);
		}
	}

	// 3. pass the point cloud to FindSurface.
	cleanUpFindSurface(fs);
	setPointCloudFloat(fs, depth_points.data(), unsigned int(depth_points.size()), 0);

	// 4. camera update
	trackball.update(time_elapsed);
	trackball2.update(time_elapsed);
}

void Application::render(int frame, double time_elapsed) {

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (screen_mode) {
	case SCREEN_MODE::DEPTH: 
		render_depth(); 
		//render_touch_point();
		break;

	case SCREEN_MODE::COLOR: 
		render_color(); 
		
		glEnable(GL_SCISSOR_TEST);
		glViewport(0, 0, width / 5, height / 5);
		glScissor(0, 0, width / 5, height / 5);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);

		render_inlier();
		render_geometry();
		
		break;

	case SCREEN_MODE::OBJECT:
		render_inlier();
		render_geometry();

		glViewport(0, 0, width / 5, height / 5);
		glDisable(GL_DEPTH_TEST);
		render_color();
		glEnable(GL_DEPTH_TEST);
	}
}

void Application::render_depth() {
	depth_renderer.position_buffer.Data(depth_points.size(), sizeof(rs::float3), depth_points.data(), GL_STREAM_DRAW);
	depth_renderer.color_buffer.Data(depth_colors.size(), sizeof(ubyte3), depth_colors.data(), GL_STREAM_DRAW);

	depth_renderer.view_matrix = trackball.view_matrix();
	depth_renderer.projection_matrix = trackball.projection_matrix();
	depth_renderer.render();
}

void Application::render_color() {
	image_renderer.render(color_intrin.width, color_intrin.height, color_image);
}

void Application::render_inlier() {
	inlier_renderer.view_matrix = trackball2.view_matrix();
	inlier_renderer.projection_matrix = trackball2.projection_matrix();
	inlier_renderer.render();
}

void Application::render_geometry() {

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	switch (result.type) {
	case FS_FEATURE_TYPE::FS_TYPE_PLANE:
		plane_renderer.view_matrix = trackball2.view_matrix();
		plane_renderer.projection_matrix = trackball2.projection_matrix();
		plane_renderer.render(); 
		break;
	case FS_FEATURE_TYPE::FS_TYPE_SPHERE:
		sphere_renderer.view_matrix = trackball2.view_matrix();
		sphere_renderer.projection_matrix = trackball2.projection_matrix();
		sphere_renderer.render();
		break;
	case FS_FEATURE_TYPE::FS_TYPE_CYLINDER:
		cylinder_renderer.view_matrix = trackball2.view_matrix();
		cylinder_renderer.projection_matrix = trackball2.projection_matrix();
		cylinder_renderer.render();
		break;
	case FS_FEATURE_TYPE::FS_TYPE_CONE:
		cone_renderer.view_matrix = trackball2.view_matrix();
		cone_renderer.projection_matrix = trackball2.projection_matrix();
		cone_renderer.render();
		break;
	case FS_FEATURE_TYPE::FS_TYPE_TORUS:
		torus_renderer.view_matrix = trackball2.view_matrix();
		torus_renderer.projection_matrix = trackball2.projection_matrix();
		torus_renderer.render();
		break;
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

//void Application::render_touch_point() {
//	vertex_arrays["geometry"].Bind(true);
//	programs["sphere"].Use();
//	float radius = 0.001f;
//	getFindSurfaceParamFloat(fs, FS_PARAM_TOUCH_R, &radius);
//	programs["sphere"].UniformMatrix4fv("model_matrix", Translate(hit_position)*Scale({ radius, radius, radius }));
//	programs["sphere"].UniformMatrix4fv("view_matrix", view_matrix);
//	programs["sphere"].UniformMatrix4fv("projection_matrix", projection_matrix);
//	programs["sphere"].Uniform3f("color", 0, 1, 0);
//	draw_sphere();
//	programs["sphere"].Use(false);
//	vertex_arrays["geometry"].Bind(false);
//
//}

void Application::finalize() {
	release_FindSurface();
	release_RealSense();
	release_OpenGL();
}

void Application::on_mouse_button(GLFWwindow* window, int button, int action, int mods) {
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	x /= width;
	y /= height;

	if (screen_mode == SCREEN_MODE::COLOR) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			run_FindSurface(float(x), float(y));
		}
	}
	else {
		scamera::Trackball* t = &trackball;
		if (screen_mode == SCREEN_MODE::OBJECT) t = &trackball2;
		scamera::Trackball::Behavior b = scamera::Trackball::Behavior::NOTHING;
		if (action == GLFW_PRESS) {
			if (button == GLFW_MOUSE_BUTTON_LEFT)			b = scamera::Trackball::Behavior::ROTATING;
			else if (button == GLFW_MOUSE_BUTTON_MIDDLE)	b = scamera::Trackball::Behavior::PANNING;
			else if (button == GLFW_MOUSE_BUTTON_RIGHT)		b = scamera::Trackball::Behavior::ROLLING;
		}
		
		t->mouse(float(x), float(y), b);
	}
}

void Application::run_FindSurface(float x, float y) {
	float depth;
	int index = cast_to_point_cloud(x, y, depth);
	hit_position = reinterpret_cast<smath::float3&>(depth_points[index]);

	// point clouds tends to have measurement errors propositional to distance.
	setFindSurfaceParamFloat(fs, FS_PARAMS::FS_PARAM_ACCURACY, 0.006f + 0.002f*(depth - 1.f));

	// if succeeds, the struct "result" will be filled with data.
	int res = findSurface(fs, type, index, &result);

	switch (res) {
	case FS_NOT_FOUND:
	case FS_UNACCEPTABLE_RESULT: fprintf(stderr, "FindSurface: failed to find (%d).\n", res); return;
	case FS_LICENSE_EXPIRED: fprintf(stderr, "FindSurface: license error occurred (FS_LICENSE_EXPIRED).\n"); return;
	case FS_LICENSE_UNKNOWN: fprintf(stderr, "FindSurface: license error occurred (FS_LICENSE_UNKNOWN).\n"); return;
	}

	const unsigned char* flags = getInOutlierFlags(fs);
	int count = getInliersFloat(fs, nullptr, 0); // retrieve the number of inlier points;
	inlier_points.clear(); inlier_points.reserve(count);
	inlier_colors.clear(); inlier_colors.reserve(count);

	for (int k = 0; k<int(getPointCloudCount(fs)); k++) {
		if (!flags[k]) {
			inlier_points.push_back(depth_points[k]);
			inlier_colors.push_back(depth_colors[k]);
		}
	}

	switch (result.type) {
	case FS_FEATURE_TYPE::FS_TYPE_PLANE:
	{
		plane_renderer.quad[0] = result.plane_param.ll;
		plane_renderer.quad[1] = result.plane_param.lr;
		plane_renderer.quad[2] = result.plane_param.ur;
		plane_renderer.quad[3] = result.plane_param.ul;
		
		using namespace smath;
		float3 ll = ToFloat3(result.plane_param.ll);
		float3 lr = ToFloat3(result.plane_param.lr);
		float3 ul = ToFloat3(result.plane_param.ul);
		float3 ur = ToFloat3(result.plane_param.ur);
		float3 center = (ll + lr + ul + ur)*0.25f;
		float3 hori = ul - ll;
		float3 vert = lr - ll;
		float width = Length(hori);
		float height = Length(vert);
		float3 normal = Normalize(Cross(hori, vert));

		fprintf(stdout, "Plane.width=%6.4f\n", width);
		fprintf(stdout, "     .height=%6.4f\n", height);
		fprintf(stdout, "     .center=<%6.4f, %6.4f, %6.4f>\n", center[0], center[1], center[2]);
		fprintf(stdout, "     .normal=<%6.4f, %6.4f, %6.4f>\n", normal[0], normal[1], normal[2]);
		break;
	}
	case FS_FEATURE_TYPE::FS_TYPE_SPHERE:
	{
		using namespace smath;
		sphere_renderer.model_matrix = Translate(smath::ToFloat3(result.sphere_param.c))*Scale(result.sphere_param.r);
		float radius = result.sphere_param.r;
		float3 center = ToFloat3(result.sphere_param.c);

		fprintf(stdout, "Sphere.radius=%6.4f\n", radius);
		fprintf(stdout, "      .center=<%6.4f, %6.4f, %6.4f>\n", center[0], center[1], center[2]);
		break;
	}
	case FS_FEATURE_TYPE::FS_TYPE_CYLINDER:
	{
		using namespace smath;
		float3 bottom_center = ToFloat3(result.cylinder_param.b);
		float3 top_center = ToFloat3(result.cylinder_param.t);
		float height = Length(top_center - bottom_center);
		float radius = result.cylinder_param.r;
		float3 center = (bottom_center + top_center)*0.5f;
		float3 cylinder_axis = Normalize(bottom_center - top_center);
		float3 y_axis = { 0, 1, 0 };
		static float deg1 = PI / 180.f;
		float3 tilt_axis = Normalize(Cross(y_axis, cylinder_axis));
		float angle = PositiveAngleBetween(y_axis, cylinder_axis, tilt_axis);

		mat4 model_matrix = Scale({ radius, height, radius });
		if (angle > deg1) model_matrix = Rotate(tilt_axis, angle)*model_matrix;
		model_matrix = Translate(center)*model_matrix;

		cylinder_renderer.model_matrix = model_matrix;

		fprintf(stdout, "Cylinder.radius=%6.4f\n", radius);
		fprintf(stdout, "        .height=%6.4f\n", height);
		fprintf(stdout, "        .center=<%6.4f, %6.4f, %6.4f>\n", center[0], center[1], center[2]);
		fprintf(stdout, "        .axis=<%6.4f, %6.4f, %6.4f>\n", cylinder_axis[0], cylinder_axis[1], cylinder_axis[2]);
		break;
	}
	case FS_FEATURE_TYPE::FS_TYPE_CONE:
	{
		using namespace smath;
		float3 bottom_center = ToFloat3(result.cone_param.b);
		float3 top_center = ToFloat3(result.cone_param.t);
		float bottom_radius = result.cone_param.br;
		float top_radius = result.cone_param.tr;
		float height = Length(top_center - bottom_center);
		float3 center = (bottom_center + top_center)*0.5f;
		float3 cone_axis = Normalize(bottom_center - top_center);
		float3 y_axis = { 0, 1, 0 };
		static float deg1 = PI / 180;
		float angle = AngleBetween(y_axis, cone_axis, Normalize(Cross(y_axis, cone_axis)));

		mat4 model_matrix = Scale({ 1, height * 0.5f, 1 });
		if (angle > deg1) model_matrix = Rotate(Normalize(Cross(y_axis, cone_axis)), angle)*model_matrix;
		model_matrix = Translate(center)*model_matrix;
	
		cone_renderer.model_matrix = model_matrix;
		cone_renderer.bottom_radius = bottom_radius;
		cone_renderer.top_radius = top_radius;

		fprintf(stdout, "Cone.top.radius=%6.4f\n", top_radius);
		fprintf(stdout, "    .bottom.radius=%6.4f\n", bottom_radius);
		fprintf(stdout, "    .height=%6.4f\n", height);
		fprintf(stdout, "    .center=<%6.4f, %6.4f, %6.4f>\n", center[0], center[1], center[2]);
		fprintf(stdout, "    .axis=<%6.4f, %6.4f, %6.4f>\n", cone_axis[0], cone_axis[1], cone_axis[2]);
		break;
	}
	case FS_FEATURE_TYPE::FS_TYPE_TORUS:
	{
		static auto get_inlier_point_cloud = [&]()->std::vector<smath::float3> {
			const unsigned char* flags = getInOutlierFlags(fs);
			int count = getInliersFloat(fs, nullptr, 0);
			std::vector<smath::float3> inliers(count);
			getInliersFloat(fs, inliers.data(), count * sizeof(smath::float3));

			return inliers;
		};

		static auto get_elbow_joint_angle = [&](smath::float3 torus_center, smath::float3 torus_axis, smath::float3& elbow_begin, float& angle) {
			using namespace smath;
			std::vector<float3>& inliers = get_inlier_point_cloud();

			for (float3& pt : inliers) {
				// point => vector (center -> point)
				float3 v = pt - torus_center;
				// project(point, plane perpendicular to the torus axis and including the torus center)
				pt = Normalize(Cross(Cross(torus_axis, v), torus_axis));
			}

			float3 barycentric = std::accumulate(inliers.begin(), inliers.end(), float3(), [](float3& i0, float3& i1) { return i0 + i1; }) / float(inliers.size());

			float3 elbow_middle = Normalize(barycentric);

			// find two extreme ends of the vectors in terms of the angle to the elbow_middle.
			std::vector<float> angles;
			angles.reserve(inliers.size());
			for (float3& pt : inliers) {
				angles.push_back(AngleBetween(pt, elbow_middle, torus_axis));
			}
			
			auto minmax = std::minmax_element(angles.begin(), angles.end());
			size_t min_index = minmax.first - angles.begin();
			size_t max_index = minmax.second - angles.begin();
			elbow_begin = inliers[max_index];
			float3 elbow_end = inliers[min_index];
			angle = PositiveAngleBetween(elbow_begin, elbow_end, torus_axis);
		};
		using namespace smath;
		float mean_radius = result.torus_param.mr;
		float tube_radius = result.torus_param.tr;
		float3 center = ToFloat3(result.torus_param.c);
		float3 axis = ToFloat3(result.torus_param.n);
		float3 elbow_begin;
		float angle;

		get_elbow_joint_angle(center, axis, elbow_begin, angle);

		float3 y_axis = { 0, 1, 0 };
		float3 tilt_axis = Normalize(Cross(y_axis, axis));
		float tilt_angle = AngleBetween(y_axis, axis, tilt_axis);
		mat4 tilt = Rotate(tilt_axis, tilt_angle);
		float3 tilt_elbow_begin = ToFloat3(tilt*float4{ 1, 0, 0, 0 });
		float3 rot_axis = Normalize(Cross(tilt_elbow_begin, elbow_begin));
		float rot_angle = PositiveAngleBetween(tilt_elbow_begin, elbow_begin, rot_axis);
		mat4 rot = Rotate(rot_axis, rot_angle);

		torus_renderer.model_matrix = Translate(center)*rot*tilt;
		torus_renderer.mean_radius = mean_radius;
		torus_renderer.tube_radius = tube_radius;
		torus_renderer.angle = angle;

		fprintf(stdout, "Torus.mean.radius=%6.4f\n", mean_radius);
		fprintf(stdout, "     .tube.radius=%6.4f\n", tube_radius);
		fprintf(stdout, "     .angle=%6.4f deg.\n", angle*180.f / PI);
		fprintf(stdout, "     .center=<%6.4f, %6.4f, %6.4f>\n", center[0], center[1], center[2]);
		fprintf(stdout, "     .axis=<%6.4f, %6.4f, %6.4f>\n", axis[0], axis[1], axis[2]);
		break;
	}
	}
	
	inlier_renderer.position_buffer.Data(inlier_points.size(), sizeof(rs::float3), inlier_points.data(), GL_STREAM_DRAW);
	inlier_renderer.color_buffer.Data(inlier_colors.size(), sizeof(ubyte3), inlier_colors.data(), GL_STREAM_DRAW);
}

void Application::on_cursor_pos(GLFWwindow* window, double x, double y) {
	if (screen_mode == SCREEN_MODE::DEPTH)
		trackball.motion(float(x / width), float(y / height));
	else
		trackball2.motion(float(x / width), float(y / height));
}

void Application::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, 1); break;
		case GLFW_KEY_D: screen_mode = SCREEN_MODE::DEPTH; fprintf(stdout, "Screen mode: DEPTH\n"); break;
		case GLFW_KEY_C: screen_mode = SCREEN_MODE::COLOR; fprintf(stdout, "Screen mode: COLOR\n"); break;
		case GLFW_KEY_O: screen_mode = SCREEN_MODE::OBJECT; fprintf(stdout, "Screen mode: OBJECT\n"); break;
		case GLFW_KEY_0: type = FS_FEATURE_TYPE::FS_TYPE_ANY; fprintf(stdout, "FindSurface: using FS_TYPE_ANY.\n"); break;
		case GLFW_KEY_1: type = FS_FEATURE_TYPE::FS_TYPE_PLANE; fprintf(stdout, "FindSurface: using FS_TYPE_PLANE.\n"); break;
		case GLFW_KEY_2: type = FS_FEATURE_TYPE::FS_TYPE_SPHERE; fprintf(stdout, "FindSurface: using FS_TYPE_SPHERE.\n"); break;
		case GLFW_KEY_3: type = FS_FEATURE_TYPE::FS_TYPE_CYLINDER; fprintf(stdout, "FindSurface: using FS_TYPE_CYLINIDER.\n"); break;
		case GLFW_KEY_4: type = FS_FEATURE_TYPE::FS_TYPE_CONE; fprintf(stdout, "FindSurface: using FS_TYPE_CONE.\n"); break;
		case GLFW_KEY_5: type = FS_FEATURE_TYPE::FS_TYPE_TORUS; fprintf(stdout, "FindSurface: using FS_TYPE_TORUS.\n"); break;
		case GLFW_KEY_HOME: trackball.reset(); break;
		case GLFW_KEY_END: trackball2.reset(); break;
		case GLFW_KEY_SLASH: 
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
				glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
				prompt_usage();
			}
		}
	}
}

void Application::on_wheel(GLFWwindow* window, double x, double y) {
	scamera::Trackball* t = nullptr;
	if (screen_mode == SCREEN_MODE::DEPTH) {
		t = &trackball;
	}
	else if (screen_mode == SCREEN_MODE::OBJECT) {
		t = &trackball2;
	}
	else return;

	t->zoom(float(y));
}

int Application::cast_to_point_cloud(double tx, double ty, float& depth) {
	
	// generate a ray in color space coordinates
	rs::float3 color_ray_begin = color_intrin.deproject_from_texcoord({ float(tx), float(ty) }, 0.5f);
	rs::float3 color_ray_end = color_intrin.deproject_from_texcoord({ float(tx), float(ty) }, 1.0f);

	// transform to depth space coordinates
	rs::float3 depth_ray_begin = color_to_depth.transform(color_ray_begin);
	rs::float3 depth_ray_end = color_to_depth.transform(color_ray_end);
	rs::float3 depth_ray_origin = color_to_depth.transform({});

	using namespace smath;

	float3 ray_begin = reinterpret_cast<float3&>(depth_ray_begin);
	float3 ray_end = reinterpret_cast<float3&>(depth_ray_end);
	float3 ray_direction = Normalize(ray_end - ray_begin);
	float3 ray_origin = reinterpret_cast<float3&>(depth_ray_origin);

	// a utility function for calculating minimum distance between a ray and a point.
	auto get_distance = [](	float3& o /* ray origin */,
							float3& d /* ray direction */,
							float3& p /* point */) {
		float3 po = p - o;
		return Length(po - d*Dot(d, po));
	};

	int index_min_dist = 0; // index of the point having minimum distance to the ray.
	float3 picked_point;	// the point having minimum distance to the ray.
	float min_dist = FLT_MAX; // minimum distance;

	for (int k = 0; k < int(depth_points.size()); k++) {
		float3& pt = reinterpret_cast<float3&>(depth_points[k]);
		float dist = get_distance(ray_origin, ray_direction, pt);
		if (dist < min_dist) {
			index_min_dist = k;
			min_dist = dist;
			picked_point = pt;
		}
	}

	depth = Length(picked_point - ray_origin);

	return index_min_dist;
}

void Application::prompt_usage() {
	fprintf(stdout, "Keyboard input\n");
	fprintf(stdout, "1: Plane\n");
	fprintf(stdout, "2: Sphere\n");
	fprintf(stdout, "3: Cylinder\n");
	fprintf(stdout, "4: Cone\n");
	fprintf(stdout, "5: Torus\n");
	fprintf(stdout, "D: switch to depth camera view (point cloud)\n");
	fprintf(stdout, "C: switch to color camera view (images)\n");
	fprintf(stdout, "O: switch to object view (point cloud)\n");
	fprintf(stdout, "HOME: reset depth camera view\n");
	fprintf(stdout, "END: reset object view\n");
	fprintf(stdout, "ESC: exit\n");
	fprintf(stdout, "Mouse input\n");
	fprintf(stdout, "Left click: find primitives (in color camera view)\n");
	fprintf(stdout, "Left drag: rotation (in depth camera or object view)\n");
	fprintf(stdout, "Middle drag: panning (in depth camera or object view)\n");
	fprintf(stdout, "Right drag: rolling (in depth camera or object view)\n");
	fprintf(stdout, "Wheel: zooming (in depth camera or object view)\n");
	fprintf(stdout, "?: prompt this message.\n");
}