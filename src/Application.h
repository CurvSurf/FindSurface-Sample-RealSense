#pragma once
#include <numeric>
#include <functional>

#include "3rdparty\glew-2.1.0\include\GL\glew.h"
#ifdef WIN32
#include "3rdparty\glfw-3.2.1.bin.WIN32\include\GLFW\glfw3.h"
#else
#include "3rdparty\glfw-3.2.1.bin.WIN64\include\GLFW\glfw3.h"
#endif
#include <gl\GL.h>
#include "3rdparty\librealsense\includes\rs.hpp"
#include "libFindSurface\include\FindSurface.h"

#include "smath.h"
#include "sgeometry.h"
#include "shader_resources.h"
#include "opengl_wrapper.h"
#include "camera.h"

using ubyte3 = struct { unsigned char r, g, b; };

class Application {

	// FindSurface ***************************
	FIND_SURFACE_CONTEXT fs;
	FS_FEATURE_RESULT result = {};
	FS_FEATURE_TYPE type = FS_FEATURE_TYPE::FS_TYPE_ANY;

	bool init_FindSurface();
	void release_FindSurface();

	// Intel RealSense ***************************
	rs::context ctx;
	rs::device* dev = nullptr;
	rs::intrinsics depth_intrin;
	rs::extrinsics depth_to_color;
	rs::extrinsics color_to_depth;
	rs::intrinsics color_intrin;
	float scale = 0.f;

	bool init_RealSense();
	int cast_to_point_cloud(double tx, double ty, float& depth);
	void release_RealSense();

	// data container ***************************
	std::vector<rs::float3> depth_points;
	std::vector<ubyte3> depth_colors;

	std::vector<rs::float3> inlier_points;
	std::vector<ubyte3> inlier_colors;

	uint8_t* color_image = nullptr;

	void init_data();

	// OpenGL ***************************
	std::map<const char*, sgl::Program> programs;
	std::map<const char*, sgl::VertexArray> vertex_arrays;
	std::map<const char*, sgl::VertexBuffer> vertex_buffers;
	std::map<const char*, sgl::IndexBuffer> index_buffers;
	sgl::DrawElementsBaseVertex draw_sphere, draw_cylinder, draw_torus;

	GLuint texture = 0;
	sgl::Buffer PBOs[2];

	void init_OpenGL();
	void release_OpenGL();

	smath::float3 hit_position = {};
	scamera::Trackball trackball = {};

	// GLEW, GLFW ***************************
	GLFWwindow* window = nullptr;
	int width = 1280, height = 960;
	const char* title = "FindSurface Demo (Intel RealSense Devices)";

	// circular queue for averaging frame rate (in ms.)
	int t_index = 0; 
	double ms_log[60];

	bool show_object = false;
	bool show_depth = false;

	// behaviors ***************************
	void update(int frame, double time_elapsed);
	void render(int frame, double time_elapsed);
	void render_color_image();
	void render_depth_point_cloud();
	void render_inlier_point_cloud();
	void render_geometries();
	void finalize();

public:

	// callbacks ***************************
	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void on_cursor_pos(GLFWwindow* window, double x, double y);
	void on_key(GLFWwindow* window, int key, int scancode, int action, int mods);

	// 
	bool init();
	void run();
};