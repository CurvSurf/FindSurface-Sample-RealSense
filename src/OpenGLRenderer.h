#pragma once
#include <map>
#include <GL/glew.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

struct vertex { glm::vec3 pos; glm::vec3 color; };

struct PointCloudRenderer {
	GLuint program;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint vertex_array;
	GLuint vertex_buffer;
	GLuint texture;
	GLuint vertex_count;

	void init();
	void update_vertices(size_t count, const vertex* data, GLenum usage);
	void update_texture(GLuint texture);
	void render_arrays(const glm::mat4& VP);
	void release();
};

struct ColorImageRenderer {
	GLuint program;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint vertex_array;
	GLuint texture;

	void init();
	void update(int width, int height, int channels, const unsigned char* pixel);
	void render();
	void release();
};

struct PlaneRenderer {
	GLuint program;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint vertex_array;
	GLuint uniform_buffer;
	GLsizeiptr data_size;
	
	void init();
	void update(const void* data); // vec4[] ll, lr, ul, ur;
	void render(const glm::mat4& VP, const glm::vec4& color);
	void release();
};

struct SphereRenderer {
	GLuint program;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint vertex_array;
	GLuint uniform_buffer;
	GLsizeiptr data_size;

	void init();
	void update(const void* data); // vec3 center, float radius;
	void render(const glm::mat4& VP, const glm::vec4& color);
	void release();
};

struct CylinderRenderer {
	GLuint program;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint vertex_array;
	GLuint uniform_buffer;
	GLsizeiptr data_size;

	void init();
	void update(const void* data); // vec3 top_center, float top_radius, vec3 bottom_center, float bottom_radius;
	void render(const glm::mat4& VP, const glm::vec4& color, int lsubdiv);
	void release();
};

typedef CylinderRenderer ConeRenderer;

struct TorusRenderer {
	GLuint program;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint vertex_array;
	GLuint uniform_buffer;
	GLsizeiptr data_size;

	void init();
	void update(const void* data); // vec3 center, float mean_radius, vec3 axis, float tube_radius, vec3 tube_begin, float tube_angle;
	void render(const glm::mat4& VP, const glm::vec4& color);
	void release();
};

struct FontRenderer {
	GLuint program;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint vertex_array;
	GLuint vertex_buffer;
	GLuint texture;

	FT_Library ft;
	FT_Face face;
	glm::uvec2 atlas_size;
	struct glyph_metric { glm::vec2 advance; glm::vec2 bitmap_size; glm::vec2 bitmap_offset; float tex_offset_x; };
	std::map<char, glyph_metric> glyph_metrics;

	void init();
	void render(int sx, int sy, float r, float g, float b, const char* fmt, ...);
	void release();
};

struct PanelRenderer {
	GLuint program;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint vertex_array;

	void init();
	void render(int x, int y, int width, int height, float r, float g, float b, float a);
	void release();
};