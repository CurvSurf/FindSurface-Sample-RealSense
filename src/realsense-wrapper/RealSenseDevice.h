#pragma once

#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <ctime>

#if defined(RS_COMPENSATE_LENS_AXIS_ALIGNMENT_ERROR)

#define RS_COMPENSATE_X (-0.002f)
#define RS_COMPENSATE_Y ( 0.001f)
#define RS_COMPENSATE_Z ( 0.000f)

#else

#define RS_COMPENSATE_X ( 0.000f)
#define RS_COMPENSATE_Y ( 0.000f)
#define RS_COMPENSATE_Z ( 0.000f)

#endif

struct RealSenseDevice {

	rs2::context ctx;
	rs2::device device;
	rs2::pipeline pipe;
	rs2::pipeline_profile profile;
	rs2::pointcloud pointcloud;
	rs2::points depth_points;
	rs2::frame color_frame;
	//rs2::decimation_filter filter;

	rs2_extrinsics depth_to_color;
	rs2_extrinsics color_to_depth;
	rs2_intrinsics depth_intrin;
	rs2_intrinsics color_intrin;
	
	void Init() {
		auto devices = ctx.query_devices();
		if (devices.size() == 0) throw std::runtime_error("No device detected.");

		device = devices.front();

		rs2::config config;
		config.enable_stream(rs2_stream::RS2_STREAM_DEPTH, 640, 480, rs2_format::RS2_FORMAT_ANY, 60);
		config.enable_stream(rs2_stream::RS2_STREAM_COLOR, 640, 480, rs2_format::RS2_FORMAT_ANY, 60);

		profile = pipe.start(config);
		auto depth_stream = profile.get_stream(RS2_STREAM_DEPTH);
		auto color_stream = profile.get_stream(RS2_STREAM_COLOR);

		depth_to_color = depth_stream.get_extrinsics_to(color_stream);
		color_to_depth = color_stream.get_extrinsics_to(depth_stream);

		auto depth_stream_profile = depth_stream.as<rs2::video_stream_profile>();
		auto color_stream_profile = color_stream.as<rs2::video_stream_profile>();

		depth_intrin = depth_stream_profile.get_intrinsics();
		color_intrin = color_stream_profile.get_intrinsics();

		set_seed(time(nullptr));
	}

	void Poll(int sampling_rate_level) {
		rs2::frameset frames = pipe.wait_for_frames();
		//filter.set_option(rs2_option::RS2_OPTION_FILTER_MAGNITUDE, float(sampling_rate_level));
		//depth_points = pointcloud.calculate(filter.process(frames.get_depth_frame()));
		depth_points = pointcloud.calculate(frames.get_depth_frame());
		color_frame = frames.get_color_frame();
		pointcloud.map_to(color_frame);
	}

	void Release() { pipe.stop(); }

	// camera info
	inline const char* GetCameraName() const { return device.get_info(rs2_camera_info::RS2_CAMERA_INFO_NAME); }

	// stream intrinsics
	inline int GetColorWidth() const { return color_intrin.width; }
	inline int GetColorHeight() const { return color_intrin.height; }
	inline glm::ivec2 GetColorResolution() const { return glm::ivec2(color_intrin.width, color_intrin.height); }
	inline float GetColorHFov() const { float fov[2]; rs2_fov(&color_intrin, fov); return fov[0]; }
	inline float GetColorVFov() const { float fov[2]; rs2_fov(&color_intrin, fov); return fov[1]; }
	inline glm::vec2 GetColorFov() const { float fov[2]; rs2_fov(&color_intrin, fov); return glm::vec2(fov[0], fov[1]); }

	inline int GetDepthWidth() const { return depth_intrin.width; }
	inline int GetDepthHeight() const { return depth_intrin.height; }
	inline glm::ivec2 GetDepthResolution() const { return glm::ivec2(depth_intrin.width, depth_intrin.height); }
	inline float GetDepthHFov() const { float fov[2]; rs2_fov(&depth_intrin, fov); return fov[0]; }
	inline float GetDepthVFov() const { float fov[2]; rs2_fov(&depth_intrin, fov); return fov[1]; }
	inline glm::vec2 GetDepthFov() const { float fov[2]; rs2_fov(&depth_intrin, fov); return glm::vec2(fov[0], fov[1]); }
	inline float GetDepthScale() const { return profile.get_device().first<rs2::depth_sensor>().get_depth_scale(); }

	glm::mat4 GetTransformColorToDepth() const {
		const float* R = color_to_depth.rotation;
		const float* T = color_to_depth.translation;
		return glm::mat4{
			R[0], R[1], R[2], 0,
			R[3], R[4], R[5], 0,
			R[6], R[7], R[8], 0,
			T[0] + RS_COMPENSATE_X, T[1] + RS_COMPENSATE_Y, T[2] + RS_COMPENSATE_Z, 1
		};
	}

	glm::mat4 GetTransformDepthToColor() const {
		const float* R = depth_to_color.rotation;
		const float* T = depth_to_color.translation;
		return glm::mat4{
			R[0], R[1], R[2], 0,
			R[3], R[4], R[5], 0,
			R[6], R[7], R[8], 0,
			T[0] - RS_COMPENSATE_X, T[1] - RS_COMPENSATE_Y, T[2] - RS_COMPENSATE_Z, 1
		};
	}

	void GetDepthPoints(int sampling_rate_level, int& vertex_count, glm::vec3* vertices) {

		// filtering out invalid depth points
		int valid_count = 0;
		{
			vertex_count = depth_points.size();

			const glm::vec3* read_vertex = reinterpret_cast<const glm::vec3*>(depth_points.get_vertices());

			glm::vec3* write_vertex = vertices;

			for (int k = 0; k < vertex_count; k++) {
				if (read_vertex->z) {
					*write_vertex++ = *read_vertex;
					valid_count++;
				}
				++read_vertex;
			}
		}

		// random sampling decimation
		switch (sampling_rate_level) {
		case 1:
			vertex_count = valid_count;
			break;
		case 2: {
			vertex_count = valid_count / 2;

			struct two_vertices { glm::vec3 even, odd; };
			const two_vertices* read_vertex = reinterpret_cast<const two_vertices*>(vertices);

			glm::vec3* write_vertex = vertices;

			for (int k = 0; k < vertex_count; k++) {
				*write_vertex++ = read_vertex++->even;
			}
			break;
		}
		default: {
			vertex_count = valid_count / sampling_rate_level;

			const glm::vec3* read_vertex = reinterpret_cast<const glm::vec3*>(vertices);

			glm::vec3* write_vertex = vertices;

			for (int k = 0; k < vertex_count; k++) {
				int index = get_int() % (valid_count - k);

				glm::vec3* read_vertex_target = write_vertex + index;
				std::swap(*write_vertex++, *read_vertex_target);
			}

			break;
		}
		}
	}
	void GetDepthPointsWithColors(int sampling_rate_level, int& vertex_count, void* vertices) {

		struct vertex { glm::vec3 pos; glm::vec2 tex; };

		// filtering out invalid depth points
		int valid_count = 0;
		{
			vertex_count = depth_points.size();

			const glm::vec3* read_vertex = reinterpret_cast<const glm::vec3*>(depth_points.get_vertices());
			const glm::vec2* read_texcoord = reinterpret_cast<const glm::vec2*>(depth_points.get_texture_coordinates());

			vertex* write_vertex = static_cast<vertex*>(vertices);
						
			for (int k = 0; k < vertex_count; k++) {
				if (read_vertex->z) {
					write_vertex->pos = *read_vertex;
					write_vertex->tex = *read_texcoord;
					++write_vertex;
					++valid_count;
				}
				++read_vertex;
				++read_texcoord;
			}
		}
	
		// random sampling decimation
		switch (sampling_rate_level) {
		case 1:
			vertex_count = valid_count;
			break;
		case 2: {
			vertex_count = valid_count / 2;

			struct two_vertices { vertex even, odd; };
			const two_vertices* read_vertex = reinterpret_cast<const two_vertices*>(vertices);

			vertex* write_vertex = static_cast<vertex*>(vertices);

			for (int k = 0; k < vertex_count; k++) {
				*write_vertex++ = read_vertex++->even;
			}
			break;
		}
		default: {
			vertex_count = valid_count / sampling_rate_level;

			const vertex* read_vertex = reinterpret_cast<const vertex*>(vertices);

			vertex* write_vertex = static_cast<vertex*>(vertices);

			for (int k = 0; k < vertex_count; k++) {
				int index = get_int() % (valid_count - k);

				vertex* read_vertex_target = write_vertex + index;
				std::swap(*write_vertex++, *read_vertex_target);
			}
			
			break;
		}
		}
	}

	const unsigned char* GetColorPixels(int& width, int& height, int& channels) {
		width = static_cast<rs2::video_frame>(color_frame).get_width();
		height = static_cast<rs2::video_frame>(color_frame).get_height();
		channels = color_frame.get_profile().format() == RS2_FORMAT_RGB8 ? 3 : 4;
		return static_cast<const unsigned char*>(color_frame.get_data());
	}

	inline glm::vec2 ProjectColor(const glm::vec3& point) { glm::vec2 pixel; rs2_project_point_to_pixel(glm::value_ptr(pixel), &color_intrin, glm::value_ptr(point)); return pixel; }
	inline glm::vec3 DeprojectColor(const glm::vec2& pixel, float depth) { glm::vec3 point; rs2_deproject_pixel_to_point(glm::value_ptr(point), &color_intrin, glm::value_ptr(pixel), depth); return point; }
	inline glm::vec2 ProjectDepth(const glm::vec3& point) { glm::vec2 pixel; rs2_project_point_to_pixel(glm::value_ptr(pixel), &depth_intrin, glm::value_ptr(point)); return point; }
	inline glm::vec3 DeprojectDepth(const glm::vec2& pixel, float depth) { glm::vec3 point; rs2_deproject_pixel_to_point(glm::value_ptr(point), &depth_intrin, glm::value_ptr(pixel), depth); return point; }
	inline glm::vec3 TransformColorToDepth(const glm::vec3& color_point) { glm::vec3 depth_point; rs2_transform_point_to_point(glm::value_ptr(depth_point), &color_to_depth, glm::value_ptr(color_point)); return depth_point; }
	inline glm::vec3 TransformDepthToColor(const glm::vec3& depth_point) { glm::vec3 color_point; rs2_transform_point_to_point(glm::value_ptr(color_point), &depth_to_color, glm::value_ptr(depth_point)); return color_point; }

private:
	uint64_t seed = 0;
	inline void set_seed(int64_t seed) { seed = (seed ^ 0x5DEECE66DLL) & ((1LL << 48) - 1); }
	inline unsigned int get_int() { seed = (seed * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1); return (unsigned int)(seed >> (48 - 32)); }

};