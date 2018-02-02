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
	
	glfwMakeContextCurrent(window);

	//glewExperimental = true;
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

	// program
	ShaderSource::init();
	for (const char* pname : { "plane", "sphere", "cylinder", "cone", "torus", "point_cloud", "color" }) {
		programs[pname].Init(ShaderSource::vs_src[pname], ShaderSource::fs_src[pname]);
	}

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

	draw_sphere.count = sphere_index_data.size();
	draw_sphere.indices = 0;
	draw_sphere.basevertex = 0;

	draw_cylinder.count = cylinder_index_data.size();
	draw_cylinder.indices = reinterpret_cast<GLvoid*>(draw_sphere.count * sizeof(unsigned int));
	draw_cylinder.basevertex = sphere_vertex_data.size();

	draw_torus.count = torus_index_data.size();
	draw_torus.indices = reinterpret_cast<GLvoid*>(GLsizei(draw_cylinder.indices) + draw_cylinder.count * sizeof(unsigned int));
	draw_torus.basevertex = draw_cylinder.basevertex + cylinder_vertex_data.size();

	// vao, vbo, ibo
	sgl::VertexArray& geometry_vao = vertex_arrays["geometry"];
	geometry_vao.Init();
	geometry_vao.Bind();
	
	sgl::VertexBuffer& geometry_vbo = vertex_buffers["geometry"];
	geometry_vbo.Init();
	geometry_vbo.Data(geometry_vertex_data.size(), sizeof(smath::float3), geometry_vertex_data.data(), GL_STATIC_DRAW);
	geometry_vbo.Bind();
	geometry_vao.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	sgl::IndexBuffer& geometry_ibo = index_buffers["geometry"];
	geometry_ibo.Init();
	geometry_ibo.Data(geometry_index_data.size(), sizeof(unsigned int), geometry_index_data.data(), GL_STATIC_DRAW);
	geometry_ibo.Bind();

	sgl::VertexArray& point_cloud_vao = vertex_arrays["point_cloud"];
	point_cloud_vao.Init();
	point_cloud_vao.Bind();
	
	sgl::VertexBuffer& point_cloud_point_vbo = vertex_buffers["point_cloud.point"];
	point_cloud_point_vbo.Init();
	point_cloud_point_vbo.Bind();
	point_cloud_vao.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	sgl::VertexBuffer& point_cloud_color_vbo = vertex_buffers["point_cloud.color"];
	point_cloud_color_vbo.Init();
	point_cloud_color_vbo.Bind();
	point_cloud_vao.AttribIPointer(1, 3, GL_UNSIGNED_BYTE, 0, 0);

	sgl::VertexArray& inlier_vao = vertex_arrays["inlier"];
	inlier_vao.Init();
	inlier_vao.Bind();

	sgl::VertexBuffer& inlier_point_vbo = vertex_buffers["inlier.point"];
	inlier_point_vbo.Init();
	inlier_point_vbo.Bind();
	inlier_vao.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	sgl::VertexBuffer& inlier_color_vbo = vertex_buffers["inlier.color"];
	inlier_color_vbo.Init();
	inlier_color_vbo.Bind();
	inlier_vao.AttribIPointer(1, 3, GL_UNSIGNED_BYTE, 0, 0);

	// PBOs, texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, color_intrin.width, color_intrin.height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	const GLsizeiptr buffer_size = color_intrin.width*color_intrin.height * 3;
	PBOs[0].Init(GL_PIXEL_UNPACK_BUFFER);
	PBOs[0].Data(buffer_size, nullptr, GL_STREAM_DRAW);
	PBOs[1].Init(GL_PIXEL_UNPACK_BUFFER);
	PBOs[1].Data(buffer_size, nullptr, GL_STREAM_DRAW);

	trackball.curr.eye = smath::float3{};
	trackball.curr.at = smath::float3{ 0, 0, 1 };
	trackball.curr.up = smath::float3{ 0, -1, 0 };
	trackball.curr.width = float(width)/1000;
	trackball.curr.height = float(height)/1000;
	trackball.curr.dnear = 0.001f;
	trackball.curr.dfar = 20.0f;
	trackball.prev = trackball.home = trackball.curr;
}

void Application::release_OpenGL() {

	for (std::pair<const char*, sgl::Program> prog_pair : programs) {
		prog_pair.second.Release();
	}
	programs.clear();

	for (std::pair<const char*, sgl::VertexArray> vao_pair : vertex_arrays) {
		vao_pair.second.Release();
	}
	vertex_arrays.clear();

	for (std::pair<const char*, sgl::VertexBuffer> vbo_pair : vertex_buffers) {
		vbo_pair.second.Release();
	}
	vertex_buffers.clear();

	for (std::pair<const char*, sgl::IndexBuffer> ibo_pair : index_buffers) {
		ibo_pair.second.Release();
	}
	index_buffers.clear();

	glDeleteTextures(1, &texture);

	PBOs[0].Release();
	PBOs[1].Release();
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
}

void Application::render(int frame, double time_elapsed) {

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if (show_depth) {
		render_depth_point_cloud();
	}
	else if (show_object) {
		render_inlier_point_cloud();
		render_geometries();

		glViewport(0, 0, width / 5, height / 5);
		render_color_image();
	}
	else {
		render_color_image();
		
		glEnable(GL_SCISSOR_TEST);
		glViewport(0, 0, width / 5, height / 5);
		glScissor(0, 0, width / 5, height / 5);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		render_inlier_point_cloud();
		render_geometries();
	}
}

void Application::render_color_image() {
	// 1. PBO ping pong
	static int index = 0;
	int next_index = 0;

	index = (index + 1) % 2;
	next_index = (index + 1) % 2;

	// 2. color image data captured from Intel RealSense device will be transferred from PBO to texture
	// The data was sent to PBO by the code below when the previous frame is rendered.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	PBOs[index].Bind();

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, color_intrin.width, color_intrin.height, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	// 3. data transfer from memory (CPU) to PBO (GPU)
	// Now we send new data to PBO, and it will be transferred to the texture when the next frame is rendered.
	GLsizeiptr data_size = color_intrin.width*color_intrin.height * 3;
	PBOs[next_index].Data(data_size, nullptr, GL_STREAM_DRAW);
	
	GLubyte*ptr = (GLubyte*)PBOs[next_index].Map(GL_WRITE_ONLY);
	if (ptr) {
		memcpy(ptr, color_image, data_size);
		PBOs[next_index].Unmap();
	}
	
	// 4. render the color image to screen.
	sgl::Program& program = programs["color"];
	program.Use(true);
	program.Uniform1i("color_texture", 0);
	//vertex_arrays["geometry"].Bind(); // bind dummy vao
	glDrawArrays(GL_TRIANGLES, 0, 6);

	program.Use(false);
}

void Application::render_depth_point_cloud() {
	vertex_arrays["point_cloud"].Bind(true);

	vertex_buffers["point_cloud.point"].Data(depth_points.size(), sizeof(rs::float3), depth_points.data(), GL_STREAM_DRAW);
	vertex_buffers["point_cloud.color"].Data(depth_colors.size(), sizeof(ubyte3), depth_colors.data(), GL_STREAM_DRAW);
	
	sgl::Program& program = programs["point_cloud"];
	program.Use(true);

	using namespace smath;
	GLsizei count = vertex_buffers["point_cloud.point"].count;

	mat4 view_matrix = trackball.view_matrix();
	mat4 projection_matrix = trackball.projection_matrix();

	program.UniformMatrix4fv("view_matrix", view_matrix);
	program.UniformMatrix4fv("projection_matrix", projection_matrix);

	vertex_arrays["point_cloud"].Bind(true);
	glDrawArrays(GL_POINTS, 0, count);
	vertex_arrays["point_cloud"].Bind(false);
	
	vertex_arrays["geometry"].Bind(true);
	programs["sphere"].Use();
	float radius = 0.001f;
	getFindSurfaceParamFloat(fs, FS_PARAM_TOUCH_R, &radius);
	programs["sphere"].UniformMatrix4fv("model_matrix", Translate(hit_position)*Scale({ radius, radius, radius }));
	programs["sphere"].UniformMatrix4fv("view_matrix", view_matrix);
	programs["sphere"].UniformMatrix4fv("projection_matrix", projection_matrix);
	programs["sphere"].Uniform3f("color", 0, 1, 0);
	draw_sphere();
	programs["sphere"].Use(false);
	vertex_arrays["geometry"].Bind(false);

	program.Use(false);
}

void Application::render_geometries() {
	static auto get_inlier_point_cloud = [&]()->std::vector<smath::float3> {
		const unsigned char* flags = getInOutlierFlags(fs);
		int count = getInliersFloat(fs, nullptr, 0);
		std::vector<smath::float3> inliers(count);
		getInliersFloat(fs, inliers.data(), count*sizeof(smath::float3));

		return inliers;
	};

	static auto get_angle_between = [](smath::float3 v0, smath::float3 v1, smath::float3 axis) {
		using namespace smath;
		float3 cross = Cross(v0, v1);
		if (Length(cross) < FLT_EPSILON) return 0.f;

		float sign = Dot(Normalize(axis), Normalize(cross));
		return sign*acosf(Dot(Normalize(v0), Normalize(v1)));
	};

	static auto get_elbow_joint_angle = [&](smath::float3 torus_center, smath::float3 torus_axis, smath::float3& elbow_begin, float& angle) {
		using namespace smath;
		std::vector<float3>& inliers = get_inlier_point_cloud();

		for (float3& pt : inliers) {
			// point => vector (center -> point)
			float3 v = pt - torus_center;
			// project(point, plane perpendicular to the torus axis and including the torus center)
			pt = Normalize(Cross(Cross(torus_axis, pt), torus_axis)); 
		}

		float3 barycentric = std::accumulate(inliers.begin(), inliers.end(), float3(), [](float3& i0, float3& i1) { return i0 + i1; }) / float(inliers.size());

		float3 elbow_middle = Normalize(barycentric);

		// find two extreme ends of the vectors in terms of the angle to the elbow_middle.
		auto minmax = std::minmax_element(inliers.begin(), inliers.end(), [elbow_middle](float3& i0, float3& i1) { return get_angle_between(i0, elbow_middle, Normalize(Cross(i0, elbow_middle))) < get_angle_between(i1, elbow_middle, Normalize(Cross(i1, elbow_middle))); });
		elbow_begin = minmax.first.operator*();
		float3 elbow_end = minmax.second.operator*();
		angle = get_angle_between(elbow_begin, elbow_end, Normalize(Cross(elbow_begin, elbow_end)));
	};

	using namespace smath;
	mat4 view_matrix = trackball.view_matrix();
	mat4 projection_matrix = trackball.projection_matrix();

	vertex_arrays["geometry"].Bind();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	switch (result.type) {
	case FS_FEATURE_TYPE::FS_TYPE_PLANE:
	{
		vertex_arrays["geometry"].Bind(false);
		sgl::Program& program = programs["plane"];
		program.Use();
		program.Uniform3f("color", 1, 0, 0);
		program.Uniform3fv("quad[0]", result.plane_param.ll);
		program.Uniform3fv("quad[1]", result.plane_param.lr);
		program.Uniform3fv("quad[2]", result.plane_param.ur);
		program.Uniform3fv("quad[3]", result.plane_param.ll);
		program.Uniform3fv("quad[4]", result.plane_param.ur);
		program.Uniform3fv("quad[5]", result.plane_param.ul);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		program.Use(false);
		break;
	}
	case FS_FEATURE_TYPE::FS_TYPE_SPHERE:
	{
		float r = result.sphere_param.r;
		mat4 model_matrix = Translate(ToFloat3(result.sphere_param.c))*Scale({ r, r, r });
		sgl::Program& program = programs["sphere"];
		program.Use();
		program.Uniform3f("color", 1, 0, 0);
		program.UniformMatrix4fv("model_matrix", model_matrix);
		program.UniformMatrix4fv("view_matrix", view_matrix);
		program.UniformMatrix4fv("projection_matrix", projection_matrix);

		draw_sphere();
		program.Use(false);
		break;
	}
	case FS_FEATURE_TYPE::FS_TYPE_CYLINDER:
	{
		float3 bottom_center = ToFloat3(result.cylinder_param.b);
		float3 top_center = ToFloat3(result.cylinder_param.t);
		float height = Length(top_center - bottom_center);
		float radius = result.cylinder_param.r;
		float3 center = (bottom_center + top_center)*0.5f;
		float3 cylinder_axis = Normalize(bottom_center - top_center);
		float3 y_axis = { 0, 1, 0 };
		static float deg1 = PI / 180.f;
		float angle = Dot(y_axis, cylinder_axis);

		mat4 model_matrix = Scale({ radius, height, radius });
		if (angle > deg1) model_matrix = Rotate(Normalize(Cross(y_axis, cylinder_axis)), angle)*model_matrix;
		model_matrix = Translate(center)*model_matrix;
		GLint err = glGetError();
		sgl::Program& program = programs["cylinder"];
		program.Use();  err = glGetError();
		program.Uniform3f("color", 1, 0, 0);  err = glGetError();
		program.UniformMatrix4fv("model_matrix", model_matrix);  err = glGetError();
		program.UniformMatrix4fv("view_matrix", view_matrix);  err = glGetError();
		program.UniformMatrix4fv("projection_matrix", projection_matrix);  err = glGetError();
		
		draw_cylinder();
		program.Use(false);
		break;
	}
	case FS_FEATURE_TYPE::FS_TYPE_CONE:
	{
		float3 bottom_center = ToFloat3(result.cone_param.b);
		float3 top_center = ToFloat3(result.cone_param.t);
		float bottom_radius = result.cone_param.br;
		float top_radius = result.cone_param.tr;
		float height = Length(top_center - bottom_center);
		float3 center = (bottom_center + top_center)*0.5f;
		float3 cylinder_axis = Normalize(bottom_center - top_center);
		float3 y_axis = { 0, 1, 0 };
		static float deg1 = PI / 180;
		float angle = get_angle_between(y_axis, cylinder_axis, Normalize(Cross(y_axis, cylinder_axis)));

		mat4 model_matrix = Scale({ 1, height * 0.5f, 1 });
		if (angle > deg1) model_matrix = Rotate(Normalize(Cross(y_axis, cylinder_axis)), angle)*model_matrix;
		model_matrix = Translate(center)*model_matrix;

		sgl::Program& program = programs["cone"];
		program.Use();
		program.Uniform3f("color", 1, 0, 0);
		program.Uniform1f("top_radius", top_radius);
		program.Uniform1f("bottom_radius", bottom_radius);
		program.UniformMatrix4fv("model_matrix", model_matrix);
		program.UniformMatrix4fv("view_matrix", view_matrix);
		program.UniformMatrix4fv("projection_matrix", projection_matrix);

		draw_cylinder();
		program.Use(false);
		break;
	}
	case FS_FEATURE_TYPE::FS_TYPE_TORUS:
	{
		float mean_radius = result.torus_param.mr;
		float tube_radius = result.torus_param.tr;
		float3 center = ToFloat3(result.torus_param.c);
		float3 axis = ToFloat3(result.torus_param.n);
		float3 elbow_begin;
		float angle;

		get_elbow_joint_angle(center, axis, elbow_begin, angle);

		float3 rot_axis = Normalize(Cross(float3{ 1, 0, 0 }, elbow_begin));
		float rot_angle = get_angle_between(float3{ 1, 0, 0 }, elbow_begin, rot_axis);
		mat4 model_matrix = Translate(center)*Rotate(rot_axis, rot_angle);

		sgl::Program& program = programs["torus"];
		program.Use();
		program.Uniform3f("color", 1, 0, 0);
		program.Uniform1f("mean_radius", mean_radius);
		program.Uniform1f("tube_radius", tube_radius);
		program.UniformMatrix4fv("model_matrix", model_matrix);
		program.UniformMatrix4fv("view_matrix", view_matrix);
		program.UniformMatrix4fv("projection_matrix", projection_matrix);

		float ratio = angle / (2.0f*PI);
		int 
		int elbow_index_count = int(draw_torus.count/3*ratio)*3;
		glDrawElementsBaseVertex(draw_torus.mode, elbow_index_count, draw_torus.type, draw_torus.indices, draw_torus.basevertex);
		program.Use(false);
		break;
	}
	}

	vertex_arrays["geometry"].Bind(false);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Application::render_inlier_point_cloud() {

	sgl::Program& program = programs["point_cloud"];
	program.Use(true);

	using namespace smath;
	GLsizei count = vertex_buffers["inlier.point"].count;

	mat4 view_matrix = trackball.view_matrix();
	mat4 projection_matrix = trackball.projection_matrix();

	program.UniformMatrix4fv("view_matrix", view_matrix);
	program.UniformMatrix4fv("projection_matrix", projection_matrix);

	vertex_arrays["inlier"].Bind(true);
	glDrawArrays(GL_POINTS, 0, count);
	vertex_arrays["inlier"].Bind(false);
	
	program.Use(false);
}

void Application::finalize() {
	release_FindSurface();
	release_RealSense();
	release_OpenGL();
}

void Application::on_mouse_button(GLFWwindow* window, int button, int action, int mods) {
	double x, y; float depth;
	glfwGetCursorPos(window, &x, &y);
	x /= width;
	y /= height;

	if (show_depth || show_object) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
		{
			if (action == GLFW_PRESS) trackball.mouse(float(x), float(y), scamera::Trackball::Behavior::ROTATING);
			else if (action == GLFW_RELEASE) trackball.mouse(float(x), float(y), scamera::Trackball::Behavior::NOTHING);
			break;
		}
		case GLFW_MOUSE_BUTTON_MIDDLE:
		{
			if (action == GLFW_PRESS) trackball.mouse(float(x), float(y), scamera::Trackball::Behavior::PANNING);
			else if (action == GLFW_RELEASE) trackball.mouse(float(x), float(y), scamera::Trackball::Behavior::NOTHING);
			break;
		}
		case GLFW_MOUSE_BUTTON_RIGHT:
		{
			if (action == GLFW_PRESS) trackball.mouse(float(x), float(y), scamera::Trackball::Behavior::ZOOMING);
			else if (action == GLFW_RELEASE) trackball.mouse(float(x), float(y), scamera::Trackball::Behavior::NOTHING);
			break;
		}
		}
	}
	else {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			int index = cast_to_point_cloud(x, y, depth);
			hit_position = reinterpret_cast<smath::float3&>(depth_points[index]);

			// point clouds tends to have measurement errors propositional to distance.
			setFindSurfaceParamFloat(fs, FS_PARAMS::FS_PARAM_ACCURACY, 0.006f + 0.002f*(depth - 1.f));
			printf("depth = %f\n", depth);
			float acc = 0.0f;
			getFindSurfaceParamFloat(fs, FS_PARAMS::FS_PARAM_ACCURACY, &acc);
			printf("acc = %f\n", acc);

			int res = findSurface(fs, type, index, &result);

			switch (res) {
			case FS_NOT_FOUND:
			case FS_UNACCEPTABLE_RESULT:
				fprintf(stderr, "FindSurface: failed to find (%d).\n", res); return;
			case FS_LICENSE_EXPIRED:
				fprintf(stderr, "FindSurface: license error occurred (FS_LICENSE_EXPIRED).\n"); return;
			case FS_LICENSE_UNKNOWN:
				fprintf(stderr, "FindSurface: license error occurred (FS_LICENSE_UNKNOWN).\n"); return;
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

			vertex_buffers["inlier.point"].Data(inlier_points.size(), sizeof(rs::float3), inlier_points.data(), GL_STREAM_DRAW);
			vertex_buffers["inlier.color"].Data(inlier_colors.size(), sizeof(ubyte3), inlier_colors.data(), GL_STREAM_DRAW);
		}
	}
}

void Application::on_cursor_pos(GLFWwindow* window, double x, double y) {
	trackball.motion(float(x/width), float(y/height));
}

void Application::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, 1); break;
		case GLFW_KEY_SPACE: show_object = !show_object; break;
		case GLFW_KEY_D: show_depth = !show_depth; break;
		case GLFW_KEY_0: type = FS_FEATURE_TYPE::FS_TYPE_ANY; fprintf(stdout, "FindSurface: using FS_TYPE_ANY.\n"); break;
		case GLFW_KEY_1: type = FS_FEATURE_TYPE::FS_TYPE_PLANE; fprintf(stdout, "FindSurface: using FS_TYPE_PLANE.\n"); break;
		case GLFW_KEY_2: type = FS_FEATURE_TYPE::FS_TYPE_SPHERE; fprintf(stdout, "FindSurface: using FS_TYPE_SPHERE.\n"); break;
		case GLFW_KEY_3: type = FS_FEATURE_TYPE::FS_TYPE_CYLINDER; fprintf(stdout, "FindSurface: using FS_TYPE_CYLINIDER.\n"); break;
		case GLFW_KEY_4: type = FS_FEATURE_TYPE::FS_TYPE_CONE; fprintf(stdout, "FindSurface: using FS_TYPE_CONE.\n"); break;
		case GLFW_KEY_5: type = FS_FEATURE_TYPE::FS_TYPE_TORUS; fprintf(stdout, "FindSurface: using FS_TYPE_TORUS.\n"); break;
		}
	}
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