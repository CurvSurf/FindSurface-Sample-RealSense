#pragma once
#include "smath.h"

namespace scamera {
	using namespace smath;

	struct Camera {
		float3 eye, at, up;
		float3 dir() { return eye - at; }
		float width, height; // orthographic projection
		float dnear, dfar;
		mat4 view_matrix;
		mat4 projection_matrix;

		void updateViewMatrix();
		void updateProjectionMatrix();
	};

	struct Trackball {

		static float zoom_speed;
		static float pan_speed;
		static float rotate_speed;
		static float translate_speed;

		enum class Behavior {
			NOTHING,
			ROTATING,
			PANNING,
			ZOOMING,
			ROLLING
		};

		Behavior behavior;

		struct {
			bool up, down, left, right, forward, backward;
			void reset() { up = down = left = right = forward = backward = false; }
			bool any() { return up || down || left || right || forward || backward; }
		} movement;

		Camera curr, prev, home;
		float x, y;

		void reset();
		void mouse(float x, float y, Behavior behavior);
		void motion(float x, float y);
		void update(double elapsed_time);

		void zoom(float sign);

		mat4 view_matrix() { return curr.view_matrix; }
		mat4 projection_matrix() { return curr.projection_matrix; }

	private:
		void pan(float x, float y);
		void roll(float x, float y);
		void rotate(float x, float y);
		void zoom(float x, float y);
	};

	
};