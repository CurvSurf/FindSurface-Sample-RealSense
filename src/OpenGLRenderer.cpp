#include "pch.h"
#include "OpenGLRenderer.h"
#include "OpenGLHelper.h"
#include <glm/gtc/type_ptr.hpp>

const char* pointcloud_vs = R"(
#version 430 core

in vec3 pos;
in vec2 tex;

out vec2 texcoord;

uniform mat4 VP;

void main() {
	gl_Position = VP * vec4(pos, 1);
	texcoord = tex;
}
)";

const char* pointcloud_fs = R"(
#version 430 core

in vec2 texcoord;

out vec4 fragcolor;

uniform sampler2D tex;

void main() {
	if (texcoord.x < 0.0 || texcoord.x > 1.0 || texcoord.y < 0.0 || texcoord.y > 1.0) {
		fragcolor = vec4(1, 0, 0, 1);
	}
	else {
		fragcolor = vec4(texture(tex, texcoord).xyz, 1);
	}
}
)";

void PointCloudRenderer::init() {

	vertex_shader = CreateShader(GL_VERTEX_SHADER, pointcloud_vs);
	fragment_shader = CreateShader(GL_FRAGMENT_SHADER, pointcloud_fs);

	program = CreateProgram(vertex_shader, fragment_shader);

	glGenVertexArrays(1, &vertex_array);
	glGenBuffers(1, &vertex_buffer);
	glBindVertexArray(vertex_array);
	{
		glBindVertexBuffer(0, vertex_buffer, 0, sizeof(vertex));

		glEnableVertexAttribArray(0);
		glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribBinding(0, 0);

		glEnableVertexAttribArray(1);
		glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec3));
		glVertexAttribBinding(1, 0);
	}
	glBindVertexArray(0);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void PointCloudRenderer::update_vertices(size_t count, const vertex* data, GLenum usage) {
	
	vertex_count = count;
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	{
		glBufferData(GL_ARRAY_BUFFER, count*sizeof(vertex), data, usage);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PointCloudRenderer::update_texture(GLuint texture) {

	int width, height;
	GLenum internal_format;

	glBindTexture(GL_TEXTURE_2D, texture);
	{
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width); 
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, reinterpret_cast<GLint*>(&internal_format));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, this->texture);
	{
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	glCopyImageSubData(
		      texture, GL_TEXTURE_2D, 0, 0, 0, 0,
		this->texture, GL_TEXTURE_2D, 0, 0, 0, 0,
		width, height, 1);
}

void PointCloudRenderer::render_arrays(const glm::mat4& VP) {

	glPointSize(3.f);
	glUseProgram(program);
	glBindVertexArray(vertex_array);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	{
		glUniform1i(glGetUniformLocation(program, "tex"), 0);
		glUniformMatrix4fv(glGetUniformLocation(program, "VP"), 1, GL_FALSE, glm::value_ptr(VP));

		glDrawArrays(GL_POINTS, 0, vertex_count);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glPointSize(1.f);
}

void PointCloudRenderer::release() {

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);

	glDeleteVertexArrays(1, &vertex_array);
	glDeleteBuffers(1, &vertex_buffer);
}

const char* colorimage_vs = R"(
#version 430 core

const vec2 quad_pos[6] = {
	vec2(-1, -1), vec2(1, -1), vec2(-1, 1),
	vec2(-1,  1), vec2(1, -1), vec2( 1, 1)
};
const vec2 quad_tex[6] = {
	vec2(0, 1), vec2(1, 1), vec2(0, 0),
	vec2(0, 0), vec2(1, 1), vec2(1, 0)
};

out vec2 texcoord;

void main() {
	gl_Position = vec4(quad_pos[gl_VertexID], 0, 1);
	texcoord = quad_tex[gl_VertexID];	
}
)";

const char* colorimage_fs = R"(
#version 430 core

in vec2 texcoord;

uniform sampler2D tex;

out vec4 fragcolor;

void main() {
	fragcolor = texture(tex, texcoord);	
}
)";

void ColorImageRenderer::init() {

	vertex_shader = CreateShader(GL_VERTEX_SHADER, colorimage_vs);
	fragment_shader = CreateShader(GL_FRAGMENT_SHADER, colorimage_fs);

	program = CreateProgram(vertex_shader, fragment_shader);

	glGenVertexArrays(1, &vertex_array);
	glGenTextures(1, &texture);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glBindTexture(GL_TEXTURE_2D, texture);
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ColorImageRenderer::update(int width, int height, int channels, const unsigned char* pixels) {

	glBindTexture(GL_TEXTURE_2D, texture);
	{
		switch (channels) {
		case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels); break;
		case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels); break;
		default:
			throw std::runtime_error("Unsupported pixel format.");
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ColorImageRenderer::render() {

	glUseProgram(program);
	glBindVertexArray(vertex_array);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	{
		glUniform1i(glGetUniformLocation(program, "tex"), 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void ColorImageRenderer::release() {

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);

	glDeleteVertexArrays(1, &vertex_array);
	glDeleteTextures(1, &texture);
}

const char* plane_vs = R"(
#version 430 core

struct plane_params 
{
	vec3 points[4];
};
layout(std140) uniform planes
{
	plane_params corners[32];
};

uniform int start_index = 0;
uniform mat4 VP;
const int quad_index[6] = {
	0, 1, 3, 3, 1, 2
};

void main() {
	gl_Position = VP*vec4(corners[start_index + gl_InstanceID].points[quad_index[gl_VertexID]], 1);
}
)";

const char* plane_fs = R"(
#version 430 core

uniform vec4 color;

out vec4 fragcolor;

void main() {
	fragcolor = color;
}
)";

void PlaneRenderer::init() {

	vertex_shader = CreateShader(GL_VERTEX_SHADER, plane_vs);
	fragment_shader = CreateShader(GL_FRAGMENT_SHADER, plane_fs);

	program = CreateProgram(vertex_shader, fragment_shader);

	glGenVertexArrays(1, &vertex_array);
	glGenBuffers(1, &uniform_buffer);

	data_size = sizeof(glm::vec4) * 4;

	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	{
		glBufferData(GL_UNIFORM_BUFFER, data_size, nullptr, GL_STATIC_DRAW);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void PlaneRenderer::update(const void* data) {

	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	{
		glBufferSubData(GL_UNIFORM_BUFFER, 0, data_size, data);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void PlaneRenderer::render(const glm::mat4& VP, const glm::vec4& color) {

	const int face_count = 2;
	const int vertex_count = face_count * 3;

	int cull_face = 0; glGetIntegerv(GL_CULL_FACE, &cull_face);
	glDisable(GL_CULL_FACE);
	int blend = 0; glGetIntegerv(GL_BLEND, &blend);
	glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glUseProgram(program);
	glBindVertexArray(vertex_array);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);
	{
		glUniformBlockBinding(program, glGetUniformBlockIndex(program, "planes"), 0);
		glUniformMatrix4fv(glGetUniformLocation(program, "VP"), 1, GL_FALSE, glm::value_ptr(VP));
		glUniform4fv(glGetUniformLocation(program, "color"), 1, glm::value_ptr(color));

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	}
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	if (cull_face == GL_TRUE) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if (blend == GL_TRUE) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void PlaneRenderer::release() {

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vertex_array);
	glDeleteBuffers(1, &uniform_buffer);
}

const char* sphere_vs = R"(
#version 430 core
#define PI 3.1415926535897932384626433832795

layout(std140) uniform spheres
{
	vec4 sphere_params[32];
};

uniform int subdiv;
uniform int start_index = 0;
uniform mat4 VP;
const int down_indices[6] = { 0, 1, 1, 0, 1, 0 };
const int right_indices[6] = { 0, 1, 2, 0, 2, 1 };

void main() {
	
	int order = int(mod(gl_VertexID, 6));
	int index = gl_VertexID/(subdiv*6) + gl_VertexID/6 + down_indices[order]*subdiv + right_indices[order];
	int theta_index = index / (subdiv + 1);
	int phi_index = int(mod(index - theta_index, subdiv));
	
	float phi = 2.0*float(phi_index) * PI / subdiv;
	float theta = float(theta_index) * PI / subdiv;
	float sphi = sin(phi);
	float cphi = cos(phi);
	float stheta = sin(theta);
	float ctheta = cos(theta);

	vec3 dir = vec3(stheta*cphi, ctheta, -stheta*sphi);

	vec4 sphere_param = sphere_params[gl_InstanceID + start_index];
	vec3 center = sphere_param.xyz;
	float radius = sphere_param.w;
	gl_Position = VP*vec4(center + dir * radius, 1);
}
)";

const char* sphere_fs = R"(
#version 430 core

uniform vec4 color;

out vec4 fragcolor;

void main() {
	fragcolor = color;
}
)";

void SphereRenderer::init() {

	vertex_shader = CreateShader(GL_VERTEX_SHADER, sphere_vs);
	fragment_shader = CreateShader(GL_FRAGMENT_SHADER, sphere_fs);

	program = CreateProgram(vertex_shader, fragment_shader);

	glGenVertexArrays(1, &vertex_array);
	glGenBuffers(1, &uniform_buffer);

	data_size = sizeof(glm::vec3) + sizeof(float);

	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	{
		glBufferData(GL_UNIFORM_BUFFER, data_size, nullptr, GL_STATIC_DRAW);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SphereRenderer::update(const void* data) {

	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	{
		glBufferSubData(GL_UNIFORM_BUFFER, 0, data_size, data);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SphereRenderer::render(const glm::mat4& VP, const glm::vec4& color) {

	const int subdiv = 16;
	const int face_count = 2 * subdiv * subdiv;
	const int vertex_count = face_count * 3;

	int cull_face = 0; glGetIntegerv(GL_CULL_FACE, &cull_face);
	glDisable(GL_CULL_FACE);
	int blend = 0; glGetIntegerv(GL_BLEND, &blend);
	glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glUseProgram(program);
	glBindVertexArray(vertex_array);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);
	{
		glUniformBlockBinding(program, glGetUniformBlockIndex(program, "spheres"), 0);
		glUniformMatrix4fv(glGetUniformLocation(program, "VP"), 1, GL_FALSE, glm::value_ptr(VP));
		glUniform4fv(glGetUniformLocation(program, "color"), 1, glm::value_ptr(color));
		glUniform1i(glGetUniformLocation(program, "subdiv"), subdiv);

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	}
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	if (cull_face == GL_TRUE) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if (blend == GL_TRUE) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void SphereRenderer::release() {

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vertex_array);
	glDeleteBuffers(1, &uniform_buffer);
}

const char* cylinder_vs = R"(
#version 430 core
#define PI 3.1415926535897932384626433832795
#define EPS 0.000000059604644775390625

struct cylinder_params 
{
	vec4 top;
	vec4 bottom;
};
layout(std140) uniform cylinders
{
	cylinder_params cylinder_param[8];
};

uniform int rsubdiv;
uniform int lsubdiv;
uniform int start_index = 0;
uniform mat4 VP;
const int down_indices[6] = { 0, 1, 1, 0, 1, 0 };
const int right_indices[6] = { 0, 1, 2, 0, 2, 1 };

void main() {

	int order = int(mod(gl_VertexID, 6));
	int index = gl_VertexID/(rsubdiv*6) + gl_VertexID/6 + down_indices[order]*rsubdiv + right_indices[order];
	int lindex = index / (rsubdiv + 1);
	int rindex = int(mod(index - lindex, rsubdiv));

	float z = float(lindex) / float(lsubdiv);
	float phi = 2.0*float(rindex) * PI / float(rsubdiv);
	float sphi = sin(phi);
	float cphi = cos(phi);
	
	cylinder_params cylinder = cylinder_param[gl_InstanceID + start_index];
	vec4 intp = mix(cylinder.top, cylinder.bottom, z);
	vec3 center = intp.xyz;
	float radius = intp.w;
	vec3 axis = (cylinder.top - cylinder.bottom).xyz;
	bool axis_is_x_axis = length(axis-vec3(1, 0, 0)) < EPS;
	vec3 circle_hori = mix(normalize(vec3(dot(axis.yz, axis.yz), -axis.x*axis.y, -axis.x*axis.z)), normalize(vec3(-axis.x*axis.z, -axis.y*axis.z, dot(axis.xy, axis.xy))), axis_is_x_axis.xxx);
	vec3 circle_vert = normalize(cross(circle_hori, axis));
	
	vec3 outward = radius*(cphi*circle_hori + sphi*circle_vert);
	gl_Position = VP*vec4(center + outward, 1);
}
)";

const char* cylinder_fs = R"(
#version 430 core

uniform vec4 color;

out vec4 fragcolor;

void main() {
	fragcolor = color;
}
)";

void CylinderRenderer::init() {

	vertex_shader = CreateShader(GL_VERTEX_SHADER, cylinder_vs);
	fragment_shader = CreateShader(GL_FRAGMENT_SHADER, cylinder_fs);

	program = CreateProgram(vertex_shader, fragment_shader);

	glGenVertexArrays(1, &vertex_array);
	glGenBuffers(1, &uniform_buffer);

	data_size = (sizeof(glm::vec3) + sizeof(float)) * 2;

	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	{
		glBufferData(GL_UNIFORM_BUFFER, data_size, nullptr, GL_STATIC_DRAW);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void CylinderRenderer::update(const void* data) {

	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	{
		glBufferSubData(GL_UNIFORM_BUFFER, 0, data_size, data);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void CylinderRenderer::render(const glm::mat4& VP, const glm::vec4& color, int lsubdiv) {

	const int rsubdiv = 16;
	const int face_count = 2 * rsubdiv * lsubdiv;
	const int vertex_count = face_count * 3;

	int cull_face = 0; glGetIntegerv(GL_CULL_FACE, &cull_face);
	glDisable(GL_CULL_FACE);
	int blend = 0; glGetIntegerv(GL_BLEND, &blend);
	glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glUseProgram(program);
	glBindVertexArray(vertex_array);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);
	{
		glUniformBlockBinding(program, glGetUniformBlockIndex(program, "cylinders"), 0);
		glUniformMatrix4fv(glGetUniformLocation(program, "VP"), 1, GL_FALSE, glm::value_ptr(VP));
		glUniform4fv(glGetUniformLocation(program, "color"), 1, glm::value_ptr(color));
		glUniform1i(glGetUniformLocation(program, "rsubdiv"), rsubdiv);
		glUniform1i(glGetUniformLocation(program, "lsubdiv"), lsubdiv);

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	}
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	if (cull_face == GL_TRUE) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if (blend == GL_TRUE) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void CylinderRenderer::release() {

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vertex_array);
	glDeleteBuffers(1, &uniform_buffer);
}

const char* torus_vs = R"(
#version 430 core
#define PI 3.1415926535897932384626433832795

struct torus_params
{
	vec4 center_mean_radius;
	vec4 axis_tube_radius;
	vec4 tube_begin_angle;
};

layout(std140) uniform torii
{
	torus_params torus_param[32];
};

uniform int tsubdiv;
uniform int psubdiv;
uniform int start_index = 0;
uniform mat4 VP;
const int down_indices[6] = { 0, 1, 1, 0, 1, 0 };
const int right_indices[6] = { 0, 1, 2, 0, 2, 1 };

void main() {

	int order = int(mod(gl_VertexID, 6));
	int index = gl_VertexID/(tsubdiv*6) + gl_VertexID/6 + down_indices[order]*tsubdiv + right_indices[order];
	int tindex = index / (tsubdiv + 1);
	int pindex = int(mod(index - tindex, psubdiv));

	torus_params torus = torus_param[gl_InstanceID + start_index];
	vec3 center = torus.center_mean_radius.xyz;
	float mean_radius = torus.center_mean_radius.w;
	vec3 axis = torus.axis_tube_radius.xyz;
	float tube_radius = torus.axis_tube_radius.w;
	vec3 tube_begin = torus.tube_begin_angle.xyz;
	float tube_angle = torus.tube_begin_angle.w;
	
	float theta = tube_angle * ((2.0 * float(tindex) / tsubdiv) - 1);
	float phi = 2.0*PI* float(pindex) / psubdiv;
	float stheta = sin(theta);
	float ctheta = cos(theta);
	float sphi = sin(phi);
	float cphi = cos(phi);

	vec3 ttan = normalize(cross(axis, tube_begin));
	vec3 tdir = tube_begin * ctheta + ttan * stheta;
	vec3 tpos = mean_radius * tdir;
	vec3 pdir = tdir * cphi - axis * sphi;	
	vec3 ppos = tube_radius * pdir;
	
	gl_Position = VP*vec4(center + tpos + ppos, 1);
}
)";

const char* torus_fs = R"(
#version 430 core

uniform vec4 color;

out vec4 fragcolor;

void main() {
	fragcolor = color;
}
)";

void TorusRenderer::init() {

	vertex_shader = CreateShader(GL_VERTEX_SHADER, torus_vs);
	fragment_shader = CreateShader(GL_FRAGMENT_SHADER, torus_fs);

	program = CreateProgram(vertex_shader, fragment_shader);

	glGenVertexArrays(1, &vertex_array);
	glGenBuffers(1, &uniform_buffer);

	data_size = (sizeof(glm::vec3) + sizeof(float)) * 3;

	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	{
		glBufferData(GL_UNIFORM_BUFFER, data_size, nullptr, GL_STATIC_DRAW);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void TorusRenderer::update(const void* data) {

	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	{
		glBufferSubData(GL_UNIFORM_BUFFER, 0, data_size, data);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void TorusRenderer::render(const glm::mat4& VP, const glm::vec4& color) {

	const int tsubdiv = 16;
	const int psubdiv = 16;
	const int face_count = 2 * tsubdiv * psubdiv;
	const int vertex_count = face_count * 3;

	int cull_face = 0; glGetIntegerv(GL_CULL_FACE, &cull_face);
	glDisable(GL_CULL_FACE);
	int blend = 0; glGetIntegerv(GL_BLEND, &blend);
	glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glUseProgram(program);
	glBindVertexArray(vertex_array);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);
	{
		glUniformBlockBinding(program, glGetUniformBlockIndex(program, "torii"), 0);
		glUniformMatrix4fv(glGetUniformLocation(program, "VP"), 1, GL_FALSE, glm::value_ptr(VP));
		glUniform4fv(glGetUniformLocation(program, "color"), 1, glm::value_ptr(color));
		glUniform1i(glGetUniformLocation(program, "tsubdiv"), tsubdiv);
		glUniform1i(glGetUniformLocation(program, "psubdiv"), psubdiv);

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	}
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	if (cull_face == GL_TRUE) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if (blend == GL_TRUE) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void TorusRenderer::release() {

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vertex_array);
	glDeleteBuffers(1, &uniform_buffer);
}

const char* font_vs = R"(
#version 430

in vec2 pos;
in vec2 tex;

out vec2 texcoord;

void main() {
	gl_Position = vec4(pos, 0, 1);
	texcoord = tex;
}
)";

const char* font_fs = R"(
#version 430

in vec2 texcoord;

uniform sampler2D tex;
uniform vec4 color;

out vec4 fragcolor;

void main() {
	fragcolor = vec4(1, 1, 1, texture(tex, texcoord).r) * color;
}
)";

void FontRenderer::init() {

	if (FT_Init_FreeType(&ft)) {
		throw std::runtime_error("Could not init freetype library.");
	}

	if (FT_New_Face(ft, "OpenSans-Regular.ttf", 0, &face)) {
		throw std::runtime_error("Could not open font.");
	}

	FT_Set_Pixel_Sizes(face, 0, 32);

	vertex_shader = CreateShader(GL_VERTEX_SHADER, font_vs);
	fragment_shader = CreateShader(GL_FRAGMENT_SHADER, font_fs);

	program = CreateProgram(vertex_shader, fragment_shader);

	glGenVertexArrays(1, &vertex_array);
	glGenBuffers(1, &vertex_buffer);

	glBindVertexArray(vertex_array);
	{
		glBindVertexBuffer(0, vertex_buffer, 0, sizeof(glm::vec4));
		
		glEnableVertexAttribArray(0);
		glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribBinding(0, 0);
		
		glEnableVertexAttribArray(1);
		glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2));
		glVertexAttribBinding(1, 0);
	}
	glBindVertexArray(0);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	FT_GlyphSlot g = face->glyph;
	
	for (int k = 32; k < 127; k++) {
		
		if (FT_Load_Char(face, k, FT_LOAD_RENDER)) continue;

		atlas_size.x += g->bitmap.width;
		atlas_size.y = glm::max(atlas_size.y, g->bitmap.rows);
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas_size.x, atlas_size.y, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

		GLenum unpack_alignment; glGetIntegerv(GL_UNPACK_ALIGNMENT, reinterpret_cast<GLint*>(&unpack_alignment));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		{
			unsigned int x = 0;
			for (char k = 32; k < 127; k++) {

				if (FT_Load_Char(face, k, FT_LOAD_RENDER)) continue;

				auto& gm = glyph_metrics[k];
				gm.advance = glm::vec2(g->advance.x >> 6, g->advance.y >> 6);
				gm.bitmap_size = glm::vec2(g->bitmap.width, g->bitmap.rows);
				gm.bitmap_offset = glm::vec2(g->bitmap_left, g->bitmap_top);
				gm.tex_offset_x = static_cast<float>(x) / atlas_size.x;

				glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

				x += g->bitmap.width;
			}
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, unpack_alignment);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FontRenderer::render(int sx, int sy, float r, float g, float b, const char* fmt, ...) {

	glm::ivec4 viewport;
	glGetIntegeri_v(GL_VIEWPORT, 0, glm::value_ptr(viewport));
	glm::vec2 wh(viewport.z, viewport.w);
	float scale_x = 1.f / wh.x;
	float scale_y = 1.f / wh.y;

	float x = sx * scale_x * 2.f - 1.f;
	float y = (wh.y - sy) * scale_y * 2.f - 1.f;

	// assemble text
	va_list va;
	va_start(va, fmt);

	char text[256];
	int len = snprintf(text, 256, fmt, va);

	va_end(va);

	// calculating quads
	int vertex_count = 0;
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	{
		std::vector<glm::vec4> buffer_data(len * 6);
		glm::vec4* write_data = buffer_data.data();
		const char* ch = text;
		FT_GlyphSlot g = face->glyph;
		for (int k = 0; k < len; k++) {

			const glyph_metric& gm = glyph_metrics[*ch++];

			float x2 = x + gm.bitmap_offset.x * scale_x;
			float y2 = y + gm.bitmap_offset.y * scale_y;
			float w = gm.bitmap_size.x * scale_x;
			float h = gm.bitmap_size.y * scale_y;

			x += gm.advance.x * scale_x;
			y += gm.advance.y * scale_y;

			// Skip glyphs that have no pixels
			if (!w || !h) continue;

			float tx = gm.bitmap_size.x / atlas_size.x;
			float ty = gm.bitmap_size.y / atlas_size.y;

			glm::vec4 v0(x2, y2, gm.tex_offset_x, 0);
			glm::vec4 v1(x2 + w, y2, gm.tex_offset_x + tx, 0);
			glm::vec4 v2(x2, y2 - h, gm.tex_offset_x, ty);
			glm::vec4 v3(x2 + w, y2 - h, gm.tex_offset_x + tx, ty);

			*write_data++ = v0;
			*write_data++ = v1;
			*write_data++ = v2;
			*write_data++ = v1;
			*write_data++ = v2;
			*write_data++ = v3;
			vertex_count += 6;
		}

		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*vertex_count, buffer_data.data(), GL_STATIC_DRAW);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// rendering text
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(program);
	glBindVertexArray(vertex_array);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	{
		glUniform1i(glGetUniformLocation(program, "tex"), 0);
		glUniform4f(glGetUniformLocation(program, "color"), r, g, b, 1);

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

void FontRenderer::release() {

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vertex_array);
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteTextures(1, &texture);
}

const char* panel_vs = R"(
#version 430 core

const vec2 quad_pos[6] = {
	vec2(-1, -1), vec2(1, -1), vec2(-1, 1),
	vec2(-1,  1), vec2(1, -1), vec2( 1, 1)
};

void main() {
	gl_Position = vec4(quad_pos[gl_VertexID], 0, 1);
}
)";

const char* panel_fs = R"(
#version 430 core

uniform vec4 color;

out vec4 fragcolor;

void main() {
	fragcolor = color;	
}
)";

void PanelRenderer::init() {

	vertex_shader = CreateShader(GL_VERTEX_SHADER, panel_vs);
	fragment_shader = CreateShader(GL_FRAGMENT_SHADER, panel_fs);

	program = CreateProgram(vertex_shader, fragment_shader);

	glGenVertexArrays(1, &vertex_array);
}

void PanelRenderer::render(int x, int y, int width, int height, float r, float g, float b, float a) {

	glm::ivec4 viewport; glGetIntegeri_v(GL_VIEWPORT, 0, glm::value_ptr(viewport));
	glViewport(x, viewport.w - y - height, width, height);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glUseProgram(program);
	glBindVertexArray(vertex_array);
	{
		glUniform4f(glGetUniformLocation(program, "color"), r, g, b, a);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	
	glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
}

void PanelRenderer::release() {

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vertex_array);
}