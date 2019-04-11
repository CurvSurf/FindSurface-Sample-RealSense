#pragma once
#include "appbase.h"
#include "FindSurface.hpp"
#include "FindSurfaceHelper.h"
#include "RealSenseDevice.h"
#include "OpenGLRenderer.h"
#include "Trackball.h"

class App : public AppBase {

	// FindSurface
	FindSurface*      fs;
	FS_FEATURE_RESULT result = {};
	FS_FEATURE_TYPE   type;
	plane_info        p_info;
	sphere_info       s_info;
	cylinder_info     c_info;
	cone_info         cn_info;
	torus_info        t_info;
	std::string       measurement;

	// RealSense
	RealSenseDevice rs;

	// Pointcloud
	int outlier_count;
	std::vector<vertex> outlier_vertices;
	int inlier_count;
	std::vector<vertex> inlier_vertices;

	// OpenGL
	PointCloudRenderer pc_renderer;
	ColorImageRenderer ci_renderer;
	PlaneRenderer      p_renderer;
	SphereRenderer     s_renderer;
	CylinderRenderer   c_renderer;
	ConeRenderer       cn_renderer;
	TorusRenderer      t_renderer;
	FontRenderer       f_renderer;
	PanelRenderer      pn_renderer;
	bool show_object;

	// Virtual trackball
	Trackball trackball;
	Trackball::Mode trackball_mode;

	void init(int width, int height) override;
	void onWindowResize(GLFWwindow* window, int width, int height) override;
	void onKey(GLFWwindow* window, int key, int scancode, int action, int mods) override;
	void onCursorPos(GLFWwindow* window, double x, double y) override;
	void onMouseButton(GLFWwindow* window, int button, int action, int mods) override;
	void onScroll(GLFWwindow* window, double xoffset, double yoffset) override;
	void update(float time_elapsed) override;
	void render(float time_elapsed) override;
	void cleanup() override;

	void render_colorimage();
	void render_object(const glm::mat4& VP);
	bool find_object(float x, float y);
	int cast_ray(const ray& r);

	void prompt_key_bindings_color();
	void prompt_key_bindings_object();
};