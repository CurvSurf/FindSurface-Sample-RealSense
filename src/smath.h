#pragma once
#include <array>
#include <algorithm>

namespace smath {

#ifndef PI
	static const float PI = std::acosf(-1);
#endif

#ifndef minmax
#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#endif
#ifndef clamp
#define clamp(val, lo, hi) ((val)<(lo) ? (lo) : (val) > (hi) ? (hi) : (val))
#endif
#ifndef lerp
#define lerp(a, b, x) ((1-x)*(a) + (x)*(b)) 
#endif
	using float3 = std::array<float, 3>;

	inline float3 ToFloat3(float* v) { return float3{ v[0], v[1], v[2] }; }
	inline float3 operator -(float3 v) { return float3{ -v[0], -v[1], -v[2] }; }
	inline float3 operator +(float3 v0, float3 v1) { return float3{ v0[0] + v1[0], v0[1] + v1[1], v0[2] + v1[2] }; }
	inline float3 operator +(float3 v, float f) { return float3{ v[0] + f, v[1] + f, v[2] + f }; }
	inline float3 operator +(float f, float3 v) { return (v + f); }
	inline float3 operator -(float3 v0, float3 v1) { return float3{ v0[0] - v1[0], v0[1] - v1[1], v0[2] - v1[2] }; }
	inline float3 operator -(float3 v, float f) { return float3{ v[0] - f, v[1] - f, v[2] - f }; }
	inline float3 operator -(float f, float3 v) { return float3{ f - v[0], f - v[1], f - v[2] }; }
	inline float3 operator *(float3 v0, float3 v1) { return float3{ v0[0] * v1[0], v0[1] * v1[1], v0[2] * v1[2] }; }
	inline float3 operator *(float3 v, float f) { return float3{ v[0] * f, v[1] * f, v[2] * f }; }
	inline float3 operator *(float f, float3 v) { return (v*f); }
	inline float3 operator /(float3 v0, float3 v1) { return float3{ v0[0] / v1[0], v0[1] / v1[1], v0[2] / v1[2] }; }
	inline float3 operator /(float3 v, float f) { return float3{ v[0] / f, v[1] / f, v[2] / f }; }
	inline float3 operator /(float f, float3 v) { return float3{ f / v[0], f / v[1], f / v[2] }; }
	inline float Sum(float3 v) { return v[0] + v[1] + v[2]; }
	inline float Dot(float3 v0, float3 v1) { return Sum(v0*v1); }
	inline float3 Cross(float3 v0, float3 v1) { return float3{ v0[1] * v1[2] - v0[2] * v1[1], v0[2] * v1[0] - v0[0] * v1[2], v0[0] * v1[1] - v0[1] * v1[0] }; }
	inline float Length(float3 v) { return sqrtf(Dot(v, v)); }
	inline float3 Normalize(float3 v) { return v / Length(v); }
	inline float AngleBetween(float3 v0, float3 v1, float3 axis) {
		float3 cross = Cross(v0, v1);
		if (Length(cross) < FLT_EPSILON) return 0.f;
		float sign = Dot(Normalize(axis), Normalize(cross));
		return sign*acosf(Dot(Normalize(v0), Normalize(v1)));
	}
	inline float PositiveAngleBetween(float3 v0, float3 v1, float3 axis) {
		float angle = AngleBetween(v0, v1, axis);
		return angle < 0.0f ? angle + PI*2.0f : angle;
	}

	using float4 = std::array<float, 4>;

	inline float3 ToFloat3(float4 v) { return float3{ v[0], v[1], v[2] }; }
	inline float4 ToFloat4(float3 v, float w = 0) { return float4{ v[0], v[1], v[2], w }; }
	inline float4 ToFloat4(float* v) { return float4{ v[0], v[1], v[2], v[3] }; }

	inline float4 operator -(float4 v) { return float4{ -v[0], -v[1], -v[2], -v[3] }; }
	inline float4 operator +(float4 v0, float4 v1) { return float4{ v0[0] + v1[0], v0[1] + v1[1], v0[2] + v1[2], v0[3] + v1[3] }; }
	inline float4 operator +(float4 v, float f) { return float4{ v[0] + f, v[1] + f, v[2] + f, v[3] + f }; }
	inline float4 operator +(float f, float4 v) { return (v + f); }
	inline float4 operator -(float4 v0, float4 v1) { return float4{ v0[0] - v1[0], v0[1] - v1[1], v0[2] - v1[2], v0[3] - v1[3] }; }
	inline float4 operator -(float4 v, float f) { return float4{ v[0] - f, v[1] - f, v[2] - f, v[3] - f }; }
	inline float4 operator -(float f, float4 v) { return float4{ f - v[0], f - v[1], f - v[2], f - v[3] }; }
	inline float4 operator *(float4 v0, float4 v1) { return float4{ v0[0] * v1[0], v0[1] * v1[1], v0[2] * v1[2], v0[3] * v1[3] }; }
	inline float4 operator *(float4 v, float f) { return float4{ v[0] * f, v[1] * f, v[2] * f, v[3] * f }; }
	inline float4 operator *(float f, float4 v) { return (v*f); }
	inline float4 operator /(float4 v0, float4 v1) { return float4{ v0[0] / v1[0], v0[1] / v1[1], v0[2] / v1[2], v0[3] / v1[3] }; }
	inline float4 operator /(float4 v, float f) { return float4{ v[0] / f, v[1] / f, v[2] / f, v[3] / f }; }
	inline float4 operator /(float f, float4 v) { return float4{ f / v[0], f / v[1], f / v[2], f / v[3] }; }
	inline float Sum(float4 v) { return v[0] + v[1] + v[2] + v[3]; }
	inline float Dot(float4 v0, float4 v1) { return Sum(v0*v1); }
	inline float Length(float4 v) { return sqrtf(Dot(v, v)); }
	inline float4 Normalize(float4 v) { return v / Length(v); }

	using mat3 = std::array<float, 9>;

	template <int N> inline typename std::enable_if<(N >= 0 && N < 3), float3>::type Row(mat3 m) { return ToFloat3(m.data() + N * 3); }
	template <int N> inline typename std::enable_if<(N >= 0 && N < 3), float3>::type Col(mat3 m) { return float3{ m[N], m[3 + N], m[6 + N] }; }

	inline mat3 ToMat3(float* v) { return mat3{ v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8] }; }
	inline mat3 ToMat3(float3 r0, float3 r1, float3 r2) { return mat3{ r0[0], r0[1], r0[2], r1[0], r1[1], r1[2], r2[0], r2[1], r2[2] }; }
	inline mat3 Identity3x3() { return mat3{ 1,0,0, 0,1,0, 0,0,1 }; }

	inline mat3 operator -(mat3 m) { mat3 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [](float x) { return -x; }); return mm; }
	inline mat3 operator +(mat3 m0, mat3 m1) { mat3 mm; std::transform(m0.cbegin(), m0.cend(), m1.cbegin(), mm.begin(), [](float x0, float x1) { return x0 + x1; }); return mm; }
	inline mat3 operator +(mat3 m, float f) { mat3 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [f](float x) { return x + f; }); return mm; }
	inline mat3 operator +(float f, mat3 m) { return (m + f); }
	inline mat3 operator -(mat3 m0, mat3 m1) { mat3 mm; std::transform(m0.cbegin(), m0.cend(), m1.cbegin(), mm.begin(), [](float x0, float x1) { return x0 - x1; }); return mm; }
	inline mat3 operator -(mat3 m, float f) { mat3 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [f](float x) { return x - f; }); return mm; }
	inline mat3 operator -(float f, mat3 m) { mat3 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [f](float x) { return f - x; }); return mm; }
	inline float3 operator *(mat3 m, float3 v) { return float3{ Dot(Row<0>(m), v), Dot(Row<1>(m), v), Dot(Row<2>(m), v) }; }
	inline float3 operator *(float3 v, mat3 m) { return float3{ Dot(v, Col<0>(m)), Dot(v, Col<1>(m)), Dot(v, Col<2>(m)) }; }
	inline mat3 operator *(mat3 m0, mat3 m1) { return ToMat3(Row<0>(m0)*m1, Row<1>(m0)*m1, Row<2>(m0)*m1); }
	inline mat3 operator *(mat3 m, float f) { mat3 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [f](float x) { return x + f; }); return mm; }
	inline mat3 operator *(float f, mat3 m) { return m*f; }
	inline mat3 operator /(mat3 m, float f) { mat3 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [f](float x) { return x / f; }); return mm; }
	inline mat3 operator /(float f, mat3 m) { mat3 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [f](float x) { return f / x; }); return mm; }

	inline mat3 Transpose(mat3 m) { return ToMat3(Row<0>(m), Row<1>(m), Row<2>(m)); }

	using mat4 = std::array<float, 16>;

	template <int N> inline typename std::enable_if<(N >= 0 && N < 4), float4>::type Row(mat4 m) { return ToFloat4(m.data() + N * 4); }
	template <int N> inline typename std::enable_if<(N >= 0 && N < 4), float4>::type Col(mat4 m) { return float4{ m[N], m[4 + N], m[8 + N], m[12 + N] }; }

	inline mat4 ToMat4(float* v) { return mat4{ v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15] }; }
	inline mat4 ToMat4(float4 r0, float4 r1, float4 r2, float4 r3) { return mat4{ r0[0], r0[1], r0[2], r0[3], r1[0], r1[1], r1[2], r1[3], r2[0], r2[1], r2[2], r2[3], r3[0], r3[1], r3[2], r3[3] }; }
	inline mat4 Identity4x4() { return mat4{ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }; }
	inline mat3 ToMat3(mat4 m) { return mat3{ m[0], m[1], m[2], m[4], m[5], m[6], m[8], m[9], m[10] }; }

	inline mat4 operator -(mat4 m) { mat4 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [](float x) { return -x; }); return mm; }
	inline mat4 operator +(mat4 m0, mat4 m1) { mat4 mm; std::transform(m0.cbegin(), m0.cend(), m1.cbegin(), mm.begin(), [](float x0, float x1) { return x0 + x1; }); return mm; }
	inline mat4 operator +(mat4 m, float f) { mat4 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [f](float x) { return x + f; }); return mm; }
	inline mat4 operator +(float f, mat4 m) { return (m + f); }
	inline mat4 operator -(mat4 m0, mat4 m1) { mat4 mm; std::transform(m0.cbegin(), m0.cend(), m1.cbegin(), mm.begin(), [](float x0, float x1) { return x0 - x1; }); return mm; }
	inline mat4 operator -(mat4 m, float f) { mat4 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [f](float x) { return x - f; }); return mm; }
	inline mat4 operator -(float f, mat4 m) { mat4 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [f](float x) { return f - x; }); return mm; }
	inline float4 operator *(mat4 m, float4 v) { return float4{ Dot(Row<0>(m), v), Dot(Row<1>(m), v), Dot(Row<2>(m), v), Dot(Row<3>(m), v) }; }
	inline float4 operator *(float4 v, mat4 m) { return float4{ Dot(v, Col<0>(m)), Dot(v, Col<1>(m)), Dot(v, Col<2>(m)), Dot(v, Col<3>(m)) }; }
	inline mat4 operator *(mat4 m0, mat4 m1) { return ToMat4(Row<0>(m0)*m1, Row<1>(m0)*m1, Row<2>(m0)*m1, Row<3>(m0)*m1); }
	inline mat4 operator *(mat4 m, float f) { mat4 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [f](float x) { return x + f; }); return mm; }
	inline mat4 operator *(float f, mat4 m) { return m*f; }
	inline mat4 operator /(mat4 m, float f) { mat4 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [f](float x) { return x / f; }); return mm; }
	inline mat4 operator /(float f, mat4 m) { mat4 mm; std::transform(m.cbegin(), m.cend(), mm.begin(), [f](float x) { return f / x; }); return mm; }

	inline mat4 Transpose(mat4 m) { return ToMat4(Row<0>(m), Row<1>(m), Row<2>(m), Row<3>(m)); }

	inline mat4 Translate(float3 offset) { return mat4{ 1,0,0,offset[0], 0,1,0,offset[1], 0,0,1,offset[2], 0,0,0,1 }; }
	inline mat4 Rotate(float3 axis, float angle) {
		float c = cosf(angle), s = sinf(angle), x = axis[0], y = axis[1], z = axis[2];
		return mat4{
			x*x*(1 - c) + c, x*y*(1 - c) - z*s, x*z*(1 - c) + y*s, 0.0f,
			x*y*(1 - c) + z*s, y*y*(1 - c) + c, y*z*(1 - c) - x*s, 0.0f,
			x*z*(1 - c) - y*s, y*z*(1 - c) + x*s, z*z*(1 - c) + c, 0.0f,
			0, 0, 0, 1.0f
		};
	}
	inline mat4 Scale(float scale) { return mat4{ scale,0,0,0, 0,scale,0,0, 0,0,scale,0, 0,0,0,1 }; }
	inline mat4 Scale(float3 scale) { return mat4{ scale[0],0,0,0, 0,scale[1],0,0, 0,0,scale[2],0, 0,0,0,1 }; }

	inline mat4 LookAt(float3 eye, float3 at, float3 up) {
		float3 n = Normalize(eye - at);
		float3 u = Normalize(Cross(up, n));
		float3 v = Normalize(Cross(n, u));
		return mat4{
			u[0], u[1], u[2], -Dot(eye, u),
			v[0], v[1], v[2], -Dot(eye, v),
			n[0], n[1], n[2], -Dot(eye, n),
			0, 0, 0, 1
		};
	}

	inline mat4 Perspective(float fovy, float aspectRatio, float dnear, float dfar) {
		float _11 = 1.0f / tanf(fovy*0.5f);
		float _00 = _11 / aspectRatio;
		float _22 = (dnear + dfar) / (dnear - dfar);
		float _23 = 2.0f*dnear*dfar / (dnear - dfar);
		float _32 = -1.0f;
		float _33 = 0.0f;
		return mat4{
			_00, 0, 0, 0,
			0, _11, 0, 0,
			0, 0, _22, _23,
			0, 0, _32, _33
		};
	}

	inline mat4 Orthographic(float width, float height, float dnear, float dfar) {
		float _00 = 2.f / width;
		float _11 = 2.f / height;
		float _22 = 2.f / (dnear - dfar);
		float _23 = (dnear + dfar) / (dnear - dfar);
		return mat4{
			_00, 0, 0, 0,
			0, _11, 0, 0,
			0, 0, _22, _23,
			0, 0, 0, 1
		};
	}
}