#include "shader_resources.h"

std::map<std::string, const char*> ShaderSource::vs_src;
std::map<std::string, const char*> ShaderSource::fs_src;

void ShaderSource::init() {
	vs_src.clear();
	fs_src.clear();

	vs_src["plane"] = R"(
#version 430

layout(location = 0) in vec3 pos;

uniform vec3 quad[6]; // ll lr ur ll ur ul
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
	gl_Position = projection_matrix*view_matrix*vec4(quad[gl_VertexID], 1);	
}
)";

	fs_src["plane"] = R"(
#version 430
out vec4 fragcolor;

uniform vec3 color;

void main() {
	fragcolor = vec4(color, 1);
}
)";

	vs_src["sphere"] = R"(
#version 430

layout(location = 0) in vec3 pos;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
	gl_Position = projection_matrix*view_matrix*model_matrix*vec4(pos, 1);
}
)";

	fs_src["sphere"] = R"(
#version 430

out vec4 fragcolor;

uniform vec3 color;

void main() {
	fragcolor = vec4(color, 1);
}
)";

	vs_src["cylinder"] = R"(
#version 430

layout(location = 0) in vec3 pos;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
	gl_Position = projection_matrix*view_matrix*model_matrix*vec4(pos, 1);
}
)";

	fs_src["cylinder"] = R"(
#version 430

out vec4 fragcolor;

uniform vec3 color;

void main() {
	fragcolor = vec4(color, 1);
}
)";

	vs_src["cone"] = R"(
#version 430

layout(location = 0) in vec3 pos;

uniform float bottom_radius;
uniform float top_radius;
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
	float height = (pos.y+1.0)*0.5;
	float interpolated_radius = mix(top_radius, bottom_radius, height);
	vec3 position = interpolated_radius*vec3(pos.x, 0, pos.z);
	position.y = pos.y;
	gl_Position = projection_matrix*view_matrix*model_matrix*vec4(position, 1);
}
)";

	fs_src["cone"] = R"(
#version 430

out vec4 fragcolor;

uniform vec3 color;

void main() {
	fragcolor = vec4(color, 1);
}
)";

	vs_src["torus"] = R"(
#version 430

layout(location = 0) in vec3 pos;

uniform float mean_radius;
uniform float tube_radius;
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
	vec3 toroidal_direction = normalize(vec3(pos.x, 0, pos.z));
	vec3 poloidal_direction = normalize(pos-toroidal_direction);
	vec3 position = mean_radius*toroidal_direction+tube_radius*poloidal_direction;
	gl_Position = projection_matrix*view_matrix*model_matrix*vec4(position, 1);
}
)";

	fs_src["torus"] = R"(
#version 430

out vec4 fragcolor;

uniform vec3 color;

void main() {
	fragcolor = vec4(color, 1);
}
)";

	vs_src["point_cloud"] = R"(
#version 430

layout(location = 0) in vec3 pos;
layout(location = 1) in uvec3 color;

out vec3 frag_color;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() {
	gl_PointSize = 2;
	gl_Position = projection_matrix*view_matrix*vec4(pos, 1);
	float norm = 1.0/255.0;
	frag_color=vec3(color*norm);
}
)";

	fs_src["point_cloud"] = R"(
#version 430

in vec3 frag_color;

out vec4 fragcolor;

void main() {
	fragcolor = vec4(frag_color, 1);
}
)";

	vs_src["color"] = R"(
#version 430

out vec2 tex;

void main() {
	vec2 vertices[6]; vec2 texcoord[6];
	vertices[0] = vec2(-1,  1); texcoord[0] = vec2(0, 0);
	vertices[1] = vec2(-1, -1); texcoord[1] = vec2(0, 1);
	vertices[2] = vec2( 1, -1); texcoord[2] = vec2(1, 1);
	vertices[3] = vec2(-1,  1); texcoord[3] = vec2(0, 0);
	vertices[4] = vec2( 1, -1); texcoord[4] = vec2(1, 1);
	vertices[5] = vec2( 1,  1); texcoord[5] = vec2(1, 0);
	
	tex = texcoord[gl_VertexID];
	gl_Position = vec4(vertices[gl_VertexID], 0, 1);
}
)";

	fs_src["color"] = R"(
#version 430

in vec2 tex;
out vec4 frag_color;

uniform sampler2D color_texture;

void main() {
	frag_color = texture(color_texture, tex);
}
)";

}