#pragma once
#include <FindSurface.h>
#include <stdexcept>
#include <vector>
#include <algorithm>

enum class FS_PARAM_LEVEL : int {
	FS_PARAM_LEVEL0 = 0,
	FS_PARAM_LEVEL1,
	FS_PARAM_LEVEL2,
	FS_PARAM_LEVEL3,
	FS_PARAM_LEVEL4,
	FS_PARAM_LEVEL5,
	FS_PARAM_LEVEL6,
	FS_PARAM_LEVEL7,
	FS_PARAM_LEVEL8,
	FS_PARAM_LEVEL9,
	FS_PARAM_LEVEL10 = 10,
};

static FS_PARAM_LEVEL enum_cast(int value) {
	if (value < static_cast<int>(FS_PARAM_LEVEL::FS_PARAM_LEVEL0) || value > static_cast<int>(FS_PARAM_LEVEL::FS_PARAM_LEVEL10)) throw std::runtime_error("The value (%d) could not be converted to FS_PARAM_LEVEL.");
	return static_cast<FS_PARAM_LEVEL>(value);
}

struct float3 { float x, y, z; };

class FindSurface {

	FIND_SURFACE_CONTEXT context = nullptr;
	FS_FEATURE_TYPE type = FS_FEATURE_TYPE::FS_TYPE_NONE;

public:
	FindSurface(float accuracy = 0.001f, float mean_distance = 0.001f, float touch_radius = 0.01f) {

		switch (createFindSurface(&context)) {
		case FS_OUT_OF_MEMORY:   throw std::runtime_error("FindSurface: OUT_OF_MEMORY");
		case FS_LICENSE_EXPIRED: throw std::runtime_error("FindSurface: FS_LICENSE_EXPIRED");
		case FS_LICENSE_UNKNOWN: throw std::runtime_error("FindSurface: FS_LICENSE_UNKNOWN");
		default: break;
		} // switch (createFindSurface(&context)

		set_accuracy(accuracy);
		set_mean_distance(mean_distance);
		set_touch_radius(touch_radius);
	}

	void set_type(FS_FEATURE_TYPE type) { this->type = type; }
	FS_FEATURE_TYPE get_type() const { return this->type; }
	
	void set_accuracy(float accuracy) { setFindSurfaceParamFloat(context, FS_PARAM_ACCURACY, accuracy < 0.f ? 0.f : accuracy); }
	void set_mean_distance(float mean_distance) { setFindSurfaceParamFloat(context, FS_PARAM_MEAN_DIST, mean_distance < 0.f ? 0.f : mean_distance); }
	void set_touch_radius(float touch_radius) { setFindSurfaceParamFloat(context, FS_PARAM_TOUCH_R, touch_radius < 0.f ? 0.f : touch_radius); }
	void set_cone2cylinder(float cone2cylinder) { setFindSurfaceParamFloat(context, FS_PARAM_CONE2CYLINDER, cone2cylinder < 0.f ? 0.f : cone2cylinder); }
	void set_radial_expansion(FS_PARAM_LEVEL level) { setFindSurfaceParamInt(context, FS_PARAM_RAD_EXP, static_cast<int>(level)); }
	void set_lateral_extension(FS_PARAM_LEVEL level) { setFindSurfaceParamInt(context, FS_PARAM_LAT_EXT, static_cast<int>(level)); }
	float get_accuracy() const { float accuracy = 0.f; getFindSurfaceParamFloat(context, FS_PARAM_ACCURACY, &accuracy); return accuracy; }
	float get_mean_distance() const { float mean_distance = 0.f; getFindSurfaceParamFloat(context, FS_PARAM_MEAN_DIST, &mean_distance); return mean_distance; }
	float get_touch_radius() const { float touch_radius = 0.f; getFindSurfaceParamFloat(context, FS_PARAM_TOUCH_R, &touch_radius); return touch_radius; }
	float get_cone2cylinder() const { float cone2cylinder = 0.f; getFindSurfaceParamFloat(context, FS_PARAM_CONE2CYLINDER, &cone2cylinder); return cone2cylinder; }
	FS_PARAM_LEVEL get_radial_expansion() const { int level = 0; getFindSurfaceParamInt(context, FS_PARAM_RAD_EXP, &level); return static_cast<FS_PARAM_LEVEL>(level); }
	FS_PARAM_LEVEL get_lateral_extension() const { int level = 0; getFindSurfaceParamInt(context, FS_PARAM_LAT_EXT, &level); return static_cast<FS_PARAM_LEVEL>(level); }

	int get_pointcloud_count() const {
		return getPointCloudCount(context);
	}

	void set_pointcloud(const float* pointer, unsigned int count, unsigned int stride) {
		cleanUpFindSurface(context);
		setPointCloudFloat(context, pointer, count, stride);
	}

	void set_pointcloud(const double* pointer, unsigned int count, unsigned int stride) {
		cleanUpFindSurface(context);
		setPointCloudDouble(context, pointer, count, stride);
	}

#ifdef GLM_VERSION
	void set_pointcloud(const std::vector<glm::vec3>& points) {
		cleanUpFindSurface(context);
		if(!points.empty())
			setPointCloudFloat(context, points.data(), static_cast<unsigned int>(points.size()), sizeof(glm::vec3));
	}
#endif

	FS_FEATURE_RESULT find_surface(unsigned int start_index, double threshold_torus_to_cylinder) const {

		FS_FEATURE_RESULT temp_result = {};
		temp_result.type = FS_TYPE_NONE;

		if (type == FS_TYPE_NONE) return temp_result;

		int res = findSurface(context, type, start_index, &temp_result);

		switch (res) {
		case FS_NOT_FOUND:
		case FS_UNACCEPTABLE_RESULT:
			return temp_result;
		case FS_LICENSE_EXPIRED: throw std::runtime_error("FindSurface: FS_LICENSE_EXPIRED");
		case FS_LICENSE_UNKNOWN: throw std::runtime_error("FindSurface: FS_LICENSE_UNKNOWN");
		default: /* if succeeded */ break;
		}

		/*******
		* NOTE: FindSurface may find a different feature type than you requested,
		*        when it detects a more proper type in its geometric topology.
		*       However, you MUST manually convert its type and parameters in the result.
		*/
		FS_FEATURE_RESULT result = {};

		/* If found FS_TYPE_CYLINDER, when requested FS_TYPE_CONE:
		*       Condition: result.cone_param.tr == result.cone_param
		*		How to convert:
		*           result.cylinder_param.r = result.cone_param.tr or result.cone_param.br
		*           result.cylinder_param.t = result.cone_param.t
		*           result.cylinder_param.b = result.cone_param.b
		*/
		if (temp_result.type == FS_TYPE_CONE && temp_result.cone_param.tr == temp_result.cone_param.br) {
			result = {};
			result.type = FS_TYPE_CYLINDER;
			result.rms = temp_result.rms;
			reinterpret_cast<float3&>(result.cylinder_param.b) = reinterpret_cast<float3&>(temp_result.cone_param.b);
			reinterpret_cast<float3&>(result.cylinder_param.t) = reinterpret_cast<float3&>(temp_result.cone_param.t);
			result.cylinder_param.r = temp_result.cone_param.br;
		}

		/* If found FS_TYPE_SPHERE, when requested FS_TYPE_TORUS:
		*       Condition: result.torus_param.mr == 0
		*       How to convert:
		*           result.sphere_param.r = result.torus_param.tr
		*           result.sphere_param.c = result.torus_param.c
		*/
		else if (temp_result.type == FS_TYPE_TORUS && temp_result.torus_param.mr < FLT_EPSILON) {
			result = {};
			result.type = FS_TYPE_SPHERE;
			result.rms = temp_result.rms;
			reinterpret_cast<float3&>(result.sphere_param.c) = reinterpret_cast<float3&>(temp_result.torus_param.c);
			result.sphere_param.r = temp_result.torus_param.tr;
		}

		/* If found FS_TYPE_CYLINDER, when requested FS_TYPE_TORUS:
		*       Condition: result.torus_param.mr == infinity
		*       How to convert:
		*           result.cylinder_param.r = result.torus_param.tr
		*           result.cylinder_param.t = result.torus_param.n
		*           result.cylinder_param.b = result.torus_param.c
		*/
		else if (temp_result.type == FS_TYPE_TORUS && 
			    (temp_result.torus_param.mr == std::numeric_limits<float>::infinity() || 
				 temp_result.torus_param.mr > temp_result.torus_param.tr * threshold_torus_to_cylinder)) {
			result = {};
			result.type = FS_TYPE_CYLINDER;
			result.rms = temp_result.rms;
			reinterpret_cast<float3&>(result.cylinder_param.b) = reinterpret_cast<float3&>(temp_result.torus_param.c);
			reinterpret_cast<float3&>(result.cylinder_param.t) = reinterpret_cast<float3&>(temp_result.torus_param.n);
			result.cylinder_param.r = temp_result.torus_param.tr;
		}

		// Otherwise, 
		else {
			result = temp_result;
		}

		/* Additionally, if found FS_TYPE_CONE,
		*  we swap the centers so that the axis (top_center - bottom_center) always points to its vertex.
		*       Condition: result.cone_param.tr > result.cone_param.br
		*       How to convert:
		*           swap(result.cone_param.tr, result.cone_param.br)
		*           swap(result.cone_param.t, result.cone_param.b)
		*/
		if (temp_result.type == FS_TYPE_CONE && temp_result.cone_param.tr > temp_result.cone_param.br) {
			result = {};
			result.type = FS_TYPE_CONE;
			result.rms = temp_result.rms;
			result.cone_param.tr = temp_result.cone_param.br;
			result.cone_param.br = temp_result.cone_param.tr;
			reinterpret_cast<float3&>(result.cone_param.t) = reinterpret_cast<float3&>(temp_result.cone_param.b);
			reinterpret_cast<float3&>(result.cone_param.b) = reinterpret_cast<float3&>(temp_result.cone_param.t);
		}

		return result;
	}

	std::vector<unsigned char> get_outlier_flags() const {
		const unsigned char* cFlags = getInOutlierFlags(context);
		unsigned int nFlags = getPointCloudCount(context);

		std::vector<unsigned char> flags; flags.assign(cFlags, cFlags + nFlags);
		//std::vector<bool> flags; flags.reserve(nFlags);
		//for (int k = 0; k < static_cast<int>(nFlags); k++) {
		//	flags.push_back(bool(cFlags[k]));
		//}
		
		return flags;
	}

	//std::vector<unsigned int> get_inlier_indices() const {
	//	const unsigned char*
	//}

	unsigned int get_outlier_count() const {
		return getOutliersFloat(context, nullptr, 0);
	}

	unsigned int get_inlier_count() const {
		return getInliersFloat(context, nullptr, 0);
	}

	void get_inliers(int size, void* data) const {
		getInliersFloat(context, data, size);
	}

	~FindSurface() {
		cleanUpFindSurface(context);
		releaseFindSurface(context);
	}
};
