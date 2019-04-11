#pragma once
#include <stdexcept>
#include <GL/glew.h>

GLuint CreateShader(GLenum shader_type, const char* shader_source) {
	GLuint shader = glCreateShader(shader_type);

	int shader_source_length = (int)strlen(shader_source);
	glShaderSource(shader, 1, &shader_source, &shader_source_length);
	glCompileShader(shader);

	int info_log_length = 0; glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length != 0) {
		std::string info_log; info_log.resize(info_log_length);
		glGetShaderInfoLog(shader, info_log_length, nullptr, const_cast<GLchar*>(info_log.data()));
		throw std::runtime_error(info_log.c_str());
	}

	GLint shader_compile_status; glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compile_status);
	if (shader_compile_status == GL_FALSE) { glDeleteShader(shader); return 0; }

	return shader;
}

GLuint CreateProgram(GLuint vertex_shader, GLuint fragment_shader) {
	if (vertex_shader == 0 || fragment_shader == 0) return 0;

	GLuint program = glCreateProgram();

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	{
		int info_log_length = 0; glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
		if (info_log_length != 0) {
			std::string info_log; info_log.resize(info_log_length);
			glGetProgramInfoLog(program, info_log_length, nullptr, const_cast<GLchar*>(info_log.data()));
			throw std::runtime_error(info_log.c_str());
		}
	}
	GLint program_link_status; glGetProgramiv(program, GL_LINK_STATUS, &program_link_status);
	if (program_link_status == GL_FALSE) { glDeleteProgram(program); return 0; }

	glValidateProgram(program);
	{
		int info_log_length = 0; glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
		if (info_log_length != 0) {
			std::string info_log; info_log.resize(info_log_length);
			glGetProgramInfoLog(program, info_log_length, nullptr, const_cast<GLchar*>(info_log.data()));
			throw std::runtime_error(info_log.c_str());
		}
	}
	GLint program_validate_status; glGetProgramiv(program, GL_VALIDATE_STATUS, &program_validate_status);
	if (program_validate_status == GL_FALSE) { glDeleteProgram(program); return 0; }

	glUseProgram(program);

	return program;
}