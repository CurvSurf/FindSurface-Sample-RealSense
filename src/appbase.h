#pragma once
#include <GLFW/glfw3.h>

struct AppBase {

protected:
	GLFWwindow* window;
	const char* window_title;

private:
	static void onWindowResizeCallback(GLFWwindow* window, int width, int height);
	static void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void onCursorPosCallback(GLFWwindow* window, double x, double y);
	static void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void onScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	void _init(int width, int height, const char* window_title);
	virtual void init(int width, int height) = 0;
	virtual void onWindowResize(GLFWwindow* window, int width, int height) = 0;
	virtual void onKey(GLFWwindow* window, int key, int scancode, int action, int mods) = 0;
	virtual void onCursorPos(GLFWwindow* window, double x, double y) = 0;
	virtual void onMouseButton(GLFWwindow* window, int button, int action, int mods) = 0;
	virtual void onScroll(GLFWwindow* window, double xoffset, double yoffset) = 0;
	virtual void update(float time_elapsed) = 0;
	virtual void render(float time_elapsed) = 0;
	virtual void cleanup() = 0;
	void _cleanup();

public:
	void Init(int width, int height, const char* window_title);
	void Run();
};