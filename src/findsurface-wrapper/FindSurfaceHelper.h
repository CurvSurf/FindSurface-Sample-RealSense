#pragma once
#include <vector>
#include <string>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>
#include <FindSurface.h>

struct plane_info {

	glm::vec4 ll, lr, ur, ul;

	inline const void* data() const { return static_cast<const void*>(this); }
	inline glm::vec3 get_normal() const { return glm::normalize(glm::cross(get_major_edge_vector(), get_minor_edge_vector())); }
	inline glm::vec3 get_center() const { return static_cast<glm::vec3>((ll + lr + ur + ul)*0.25f); }
	inline glm::vec3 get_major_edge_vector() const { glm::vec3 edge0 = lr - ll; glm::vec3 edge1 = ul - ll; return glm::length(edge0) > glm::length(edge1) ? edge0 : edge1; }
	inline glm::vec3 get_minor_edge_vector() const { glm::vec3 edge0 = lr - ll; glm::vec3 edge1 = ul - ll; return glm::length(edge0) < glm::length(edge1) ? edge0 : edge1; }
	
	plane_info() = default;
	plane_info(const plane_info& other) = default;
	plane_info(const FS_FEATURE_RESULT& result) { *this = result; }
	plane_info& operator=(const plane_info& other) = default;
	plane_info& operator=(const FS_FEATURE_RESULT& result) { ll = glm::vec4(*reinterpret_cast<const glm::vec3*>(result.plane_param.ll), 1); lr = glm::vec4(*reinterpret_cast<const glm::vec3*>(result.plane_param.lr), 1); ul = glm::vec4(*reinterpret_cast<const glm::vec3*>(result.plane_param.ul), 1); ur = glm::vec4(*reinterpret_cast<const glm::vec3*>(result.plane_param.ur), 1); return *this; }
};

struct sphere_info {

	glm::vec3 center;
	float radius;

	inline const void* data() const { return static_cast<const void*>(this); }
	sphere_info() = default;
	sphere_info(const sphere_info& other) = default;
	sphere_info(const FS_FEATURE_RESULT& result) { *this = result; }
	sphere_info& operator=(const sphere_info& other) = default;
	sphere_info& operator=(const FS_FEATURE_RESULT& result) { center = *reinterpret_cast<const glm::vec3*>(result.sphere_param.c); radius = result.sphere_param.r; return *this; }
};

struct cylinder_info {

	glm::vec3 top_center;
	float top_radius;
	glm::vec3 bottom_center;
	float bottom_radius;

	inline const void* data() const { return static_cast<const void*>(this); }
	inline glm::vec3 get_axis() const { return glm::normalize(top_center - bottom_center); }
	inline glm::vec3 get_center() const { return glm::mix(top_center, bottom_center, 0.5f); }
	inline float get_length() const { return glm::distance(top_center, bottom_center); }

	cylinder_info() = default;
	cylinder_info(const cylinder_info& other) = default;
	cylinder_info(const FS_FEATURE_RESULT& result) { *this = result; }
	cylinder_info& operator=(const cylinder_info& other) = default;
	cylinder_info& operator=(const FS_FEATURE_RESULT& result) { top_center = *reinterpret_cast<const glm::vec3*>(result.cylinder_param.t); bottom_center = *reinterpret_cast<const glm::vec3*>(result.cylinder_param.b); top_radius = bottom_radius = result.cylinder_param.r; return *this; }
};

struct cone_info : public cylinder_info {

	inline const void* data() const { return static_cast<const void*>(this); }
	inline float get_conic_angle() const { return glm::atan(get_length(), glm::abs(top_radius - bottom_radius)); }

	cone_info() = default;
	cone_info(const cone_info& other) = default;
	cone_info(const FS_FEATURE_RESULT& result) { *this = result; }
	cone_info& operator=(const cone_info& other) = default;
	cone_info& operator=(const FS_FEATURE_RESULT& result) { top_center = *reinterpret_cast<const glm::vec3*>(result.cone_param.t); bottom_center = *reinterpret_cast<const glm::vec3*>(result.cone_param.b); top_radius = result.cone_param.tr; bottom_radius = result.cone_param.br; return *this; }
};

struct torus_info {

	glm::vec3 center;
	float mean_radius; // major radius
	glm::vec3 axis;
	float tube_radius; // minor radius
	glm::vec3 tube_begin;
	float tube_angle;

	inline const void* data() const { return static_cast<const void*>(this); }
	inline float get_tube_length() const { return mean_radius * tube_angle; }
	
	torus_info() = default;
	torus_info(const torus_info& other) = default;
	torus_info(const FS_FEATURE_RESULT& result, const std::vector<glm::vec3>& inlier_points) {
		center = *reinterpret_cast<const glm::vec3*>(result.torus_param.c);
		mean_radius = result.torus_param.mr;
		axis = *reinterpret_cast<const glm::vec3*>(result.torus_param.n);
		tube_radius = result.torus_param.tr;

		// fetch inlier points
		const int inlier_count = inlier_points.size();
		std::vector<glm::vec3> inlier_vectors(inlier_count);
		const glm::vec3* points = inlier_points.data();

		// transform the points to vectors
		glm::vec3 bc = glm::vec3();
		for (glm::vec3& pt : inlier_vectors) {
			pt = glm::normalize(glm::cross(glm::cross(axis, *(points++) - center), axis));
			bc += pt;
		}

		glm::vec3 middle = glm::normalize(bc / static_cast<float>(inlier_count));

		// get angles between middle and the points
		// and find the elements having min/max angles
		int k = 0, min_index = -1, max_index = -1;
		float min_angle = FLT_MAX;
		float max_angle = -FLT_MAX;
		for (const glm::vec3& vector : inlier_vectors) {
			float angle = glm::orientedAngle(middle, vector, axis);
			if (angle < min_angle) { min_angle = angle; min_index = k; }
			if (angle > max_angle) { max_angle = angle; max_index = k; }
			k++;
		}

		tube_angle = (max_angle - min_angle) * 0.5f;
		tube_begin = middle;
	}
	torus_info& operator=(const torus_info& other) = default;
};

//std::wstring get_enum_string(FS_FEATURE_TYPE type);

