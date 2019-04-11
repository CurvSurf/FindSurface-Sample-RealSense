#include "pch.h"
#include "appbase.h"
#include <chrono>
#include <stdexcept>

void AppBase::onWindowResizeCallback(GLFWwindow* window, int width, int height) { reinterpret_cast<AppBase*>(glfwGetWindowUserPointer(window))->onWindowResize(window, width, height); }

void AppBase::onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) { reinterpret_cast<AppBase*>(glfwGetWindowUserPointer(window))->onKey(window, key, scancode, action, mods); }

void AppBase::onCursorPosCallback(GLFWwindow* window, double x, double y) { reinterpret_cast<AppBase*>(glfwGetWindowUserPointer(window))->onCursorPos(window, x, y); }

void AppBase::onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) { reinterpret_cast<AppBase*>(glfwGetWindowUserPointer(window))->onMouseButton(window, button, action, mods); }

void AppBase::onScrollCallback(GLFWwindow* window, double xoffset, double yoffset) { reinterpret_cast<AppBase*>(glfwGetWindowUserPointer(window))->onScroll(window, xoffset, yoffset); }

void AppBase::_init(int width, int height, const char* window_title) {

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, window_title, nullptr, nullptr);

#if GLFW_MINOR_VERSION > 1
	glfwSetWindowSizeLimits(window, 640, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);
	glfwSetWindowAspectRatio(window, 4, 3);
#endif

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, AppBase::onWindowResizeCallback);
	glfwSetKeyCallback(window, AppBase::onKeyCallback);
	glfwSetCursorPosCallback(window, AppBase::onCursorPosCallback);
	glfwSetMouseButtonCallback(window, AppBase::onMouseButtonCallback);
	glfwSetScrollCallback(window, AppBase::onScrollCallback);

	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error("failed to initialize glew!");
	}
}

void AppBase::_cleanup() {

	glfwDestroyWindow(window);
	glfwTerminate();
}
typedef void(__stdcall *GLDEBUGPROC)(GLenum source, GLenum type, uint32_t id, GLenum severity, int32_t length, const char* message, const void* userParam);
void __stdcall DebugProc(GLenum source, GLenum type, uint32_t id, GLenum severity, int32_t length, const char* message, const void* userParam) {
	
	if (type != GL_DEBUG_TYPE_ERROR) return;
	printf("%s\n", message);

}
void AppBase::Init(int width, int height, const char * window_title) {

	this->window_title = window_title;
	_init(width, height, window_title);
	init(width, height);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(&DebugProc, this);
}

void AppBase::Run() {

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		static bool first_frame = true;
		static auto t0 = std::chrono::high_resolution_clock::now();
		float dt = 1000.f / 60.f;
		if (first_frame) first_frame = false;
		else {
			auto t1 = std::chrono::high_resolution_clock::now();
			dt = std::chrono::duration<float, std::chrono::milliseconds::period>(t1 - t0).count();
			t0 = t1;

			static float ema = dt;
			float weight = 2.f / (int(1000.f / ema) + 1);
			
			ema = glm::clamp(glm::mix(ema, dt, weight), 0.001f, 1000.f);
			
			int fps = int(1000.f / ema);
			char buf[128];
			int len = snprintf(buf, 60, "%s (%.2g ms, %d fps)", window_title, ema, fps);
			glfwSetWindowTitle(window, buf);
		}

		update(dt);
		render(dt);

		glfwSwapBuffers(window);
	}

	cleanup();
	_cleanup();
}