#pragma once
#include <numeric>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>

// Axis-aligned bounding box for pointcloud
struct AABB {
	glm::vec3 m = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 M = glm::vec3(-std::numeric_limits<float>::max());

	AABB() = default;
	AABB(const AABB&) = default;
	AABB(const glm::vec3& m, const glm::vec3& M) : m(m), M(M) {}
	AABB& operator=(const AABB&) = default;

	inline glm::vec3 center() const { return glm::mix(m, M, 0.5f); }
	inline glm::vec3 extent() const { return M - m; }

	bool include(glm::vec3 p) const { return glm::all(glm::lessThanEqual(m, p) && glm::lessThanEqual(p, M)); }
	void expand(glm::vec3 p) { if (!include(p)) { m = glm::min(m, p); M = glm::max(M, p); } }
};

struct ray {
	glm::vec3 org;
	glm::vec3 dir;

	inline float get_distance(glm::vec3 point) const {
		glm::vec3 T = point - org;
		float a = glm::dot(T, dir) / glm::dot(dir, dir);
		return glm::distance(T, a * dir);
	}
};

struct Camera {

	glm::vec3 eye = glm::vec3(0, 0, -1);
	glm::vec3 at = glm::vec3(0, 0, 0);
	glm::vec3 up = glm::vec3(0, 1, 0);
	float width = 1;
	float height = 1;
	float fovy = 0.f;
	float zoom_factor = 1.f;
	float dnear = 0.001f;
	float dfar = 20.f;

	glm::mat4 view_matrix;
	glm::mat4 projection_matrix;

	Camera() = default;
	Camera(const Camera&) = default;
	Camera& operator=(const Camera&) = default;

	inline glm::vec3 dir() const { return at - eye; }
	inline float aspect_ratio() const { return width / height; }

	void update_view_matrix() { 
		if (fovy < std::numeric_limits<float>::epsilon()) {
			view_matrix = glm::lookAt(eye, at, up);
		}
		else {
			view_matrix = glm::lookAt(eye + dir() / zoom_factor, at, up);
		}
	}

	void update_proj_matrix() { 
		if (fovy < std::numeric_limits<float>::epsilon()) {
			float hw = 0.5f*width / zoom_factor;
			float hh = 0.5f*height / zoom_factor;
			projection_matrix = glm::ortho(-hw, hw, -hh, hh, dnear, dfar);
			
		}
		else {
			projection_matrix = glm::perspective(fovy, aspect_ratio(), dnear, dfar);
			
		}
	}
};

struct Trackball {

	constexpr static float zoom_speed = 0.05f;
	constexpr static float pan_speed = 0.1f;
	constexpr static float rotate_speed = 0.75f;
	constexpr static float translate_speed = 0.0005f;

	enum Mode {
		NONE,
		ROTATE,
		PAN,
		ZOOM,
		ROLL
	} mode;
	Camera curr, prev, home;
	glm::vec2 m0;

	void reset() {
		curr = prev = home;
	}

	void reset(const Camera& cam) {
		curr = prev = home = cam;
	}

	void mouse(float x, float y, Mode mode) {
		prev = curr;
		m0 = normalize(x, y);
		this->mode = mode;
	}

	void motion(float x, float y) {
		glm::vec2 m = normalize(x, y);
		if (glm::length(m - m0) < std::numeric_limits<float>::epsilon()) return;

		switch (mode) {
		case PAN:    pan(m); break;
		case ROLL:   roll(m); break;
		case ROTATE: rotate(m); break;
		case ZOOM:   zoom(m); break;
		default: break;
		}
	}

	void update() {
		curr.update_view_matrix();
		curr.update_proj_matrix();
	}

	glm::mat4 view_matrix() const {
		return curr.view_matrix;
	}

	glm::mat4 projection_matrix() const {
		return curr.projection_matrix;
	}

	glm::vec2 normalize(float x, float y) const {
		x /= prev.width;
		y /= prev.height;
		x = 2 * x - 1;
		y = 1 - 2 * y;
		x = glm::clamp(x, -1.f, 1.f);
		y = glm::clamp(y, -1.f, 1.f);
		return glm::vec2(x, y);
	}

private:
	void pan(glm::vec2 m) {
		
		glm::vec2 dm = m - m0;
#if defined(GLM_FORCE_LEFT_HANDED)
		dm = -dm;
#endif
		if (curr.fovy < std::numeric_limits<float>::epsilon()) {
			dm *= glm::vec2(prev.width, prev.height) * 0.5f / prev.zoom_factor;
		}
		else {
			float scale = glm::length(prev.dir() / prev.zoom_factor) * pan_speed;
			dm *= scale;
		}

		glm::vec3 n = glm::normalize(-prev.dir());
		glm::vec3 u = glm::normalize(glm::cross(prev.up, n));
		glm::vec3 v = glm::normalize(glm::cross(n, u));
		glm::vec3 d = u * dm.x + v * dm.y;

		curr.eye = prev.eye - d;
		curr.at = prev.at - d;

		curr.update_view_matrix();
	}

	void roll(glm::vec2 m) {
		// not working
		//float angle = glm::orientedAngle(m0, m);
		//
		//glm::vec3 n = glm::normalize(-prev.dir());
		//glm::vec3 u = glm::normalize(glm::cross(prev.up, n));
		//glm::vec3 v = glm::normalize(glm::cross(n, u));
		//
		//curr.up = glm::rotate(angle, n) * glm::vec4(v, 0);
		//curr.update_view_matrix();
	}

	void rotate(glm::vec2 m) {
		glm::vec2 dm = (m - m0) * rotate_speed;
#if defined(GLM_FORCE_LEFT_HANDED)
		dm = -dm;
#endif
		float z = 1 - glm::length2(dm);
		glm::vec3 p1 = { dm, glm::sqrt(glm::max(0.f, z)) };
		glm::vec3 n = glm::cross(glm::vec3{ 0, 0, 1 }, p1)*glm::mat3(prev.view_matrix);
		float len = glm::length(n);
		float angle = asinf(glm::min(len, 0.999f));

		glm::mat4 T = glm::translate(prev.at);
		glm::mat4 R = glm::rotate(-angle, glm::normalize(n));

		curr.eye = (T*R*glm::vec4(-prev.dir(), 1));
		curr.at = prev.at;
		curr.up = (R*glm::vec4(prev.up, 0));
		curr.update_view_matrix();
	}

	void zoom(glm::vec2 m) {
		float dt = glm::pow(2.f, m.x - m0.x);
		curr.zoom_factor = prev.zoom_factor * dt;

		if (curr.fovy < std::numeric_limits<float>::epsilon()) {
			curr.update_proj_matrix();
		}
		else {
			curr.update_view_matrix();
		}
	}
};