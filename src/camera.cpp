#include "camera.h"

namespace scamera {

	float Trackball::zoom_speed = 0.05f;
	float Trackball::pan_speed = 0.1f;
	float Trackball::rotate_speed = 0.75f;
	float Trackball::translate_speed = 0.0005f;

	void Camera::updateViewMatrix() { view_matrix = LookAt(eye, at, up); }
	void Camera::updateProjectionMatrix() { projection_matrix = Orthographic(width, height, dnear, dfar); }


	void Trackball::reset() {
		curr = prev = home;
	}

	void Trackball::mouse(float x, float y, Trackball::Behavior behavior) {
		prev = curr;
		x = 2 * x - 1;
		y = 1 - 2 * y;
		this->x = clamp(x, -1, 1);
		this->y = clamp(y, -1, 1);
		this->behavior = behavior;
	}

	void Trackball::motion(float x, float y) {
		x = 2 * x - 1;
		y = 1 - 2 * y;
		x = clamp(x, -1, 1);
		y = clamp(y, -1, 1);

		switch (behavior) {
		case Trackball::Behavior::PANNING: pan(x, y); break;
		case Trackball::Behavior::ROLLING: roll(x, y); break;
		case Trackball::Behavior::ROTATING: rotate(x, y); break;
		case Trackball::Behavior::ZOOMING: zoom(x, y); break;
		}
	}

	void Trackball::update(double elapsed_time) {
		if (movement.any()) {
			float distance = float(Length(home.dir())*elapsed_time*translate_speed);
			float3 n = -Normalize(curr.dir());
			float3 u = Normalize(Cross(curr.up, n));
			float3 v = Normalize(Cross(n, u));

			if (movement.forward) { curr.eye = curr.eye + n*distance; }
			if (movement.backward) { curr.eye = curr.eye - n*distance; }
			if (movement.left) { curr.eye = curr.eye - u*distance; }
			if (movement.right) { curr.eye = curr.eye + u*distance; }
			if (movement.up) { curr.eye = curr.eye + v*distance; }
			if (movement.down) { curr.eye = curr.eye - v*distance; }
		}

		curr.updateViewMatrix();
		curr.updateProjectionMatrix();
	}

	void Trackball::pan(float x, float y) {
		float scale = Length(curr.dir())*pan_speed;
		float dx = (x - this->x)*scale;
		float dy = (y - this->y)*scale;

		float3 n = Normalize(prev.dir());
		float3 u = Normalize(Cross(prev.up, n));
		float3 v = Normalize(Cross(n, u));
		float3 d = u*dx + v*dy;

		curr.eye = prev.eye - d;
		curr.at = prev.at - d;

		curr.updateViewMatrix();
	}

	void Trackball::roll(float x, float y) {
		float dx = (x - this->x)*rotate_speed;
		float dy = (y - this->y)*rotate_speed;

		float3 n = Normalize(prev.dir());
		float3 u = Normalize(Cross(prev.up, n));
		float3 v = Normalize(Cross(n, u));

		curr.up = ToFloat3(Rotate(n, dy*100.f*PI / 180.f)*(ToFloat4(v, 0)));
		curr.updateViewMatrix();
	}

	void Trackball::rotate(float x, float y) {
		x = (x - this->x)*rotate_speed;
		y = (y - this->y)*rotate_speed;
		float z = 1 - (x*x + y*y);
		float3 p1 = { x, y, (float)sqrtf(max(0, z)) };
		float3 n = Cross(float3{ 0, 0, 1 }, p1)*ToMat3(prev.view_matrix);
		float len = Length(n);
		float angle = asinf(min(len, 0.999f));

		mat4 T = Translate(prev.at);
		mat4 R = Rotate(Normalize(n), -angle);

		curr.eye = ToFloat3(T*R*ToFloat4(prev.dir(), 1));
		curr.at = prev.at;
		curr.up = ToFloat3(R*ToFloat4(prev.up, 0));
		curr.updateViewMatrix();
	}

	void Trackball::zoom(float x, float y) {
		float sign = y < this->y ? 1.f : -1.f;
		this->y = y;
		this->x = x;
		float dt = 1.f+sign*zoom_speed;
		curr.width *= dt;
		curr.height *= dt;
		curr.updateProjectionMatrix();
	}

	void Trackball::zoom(float sign) {
		float dt = 1.f + sign*zoom_speed;
		curr.width *= dt;
		curr.height *= dt;
		curr.updateProjectionMatrix();
	}
}