#include "opengl_wrapper.h"

namespace sgl {

	GLuint CreateShader(GLenum shader_type, const char* shader_source) {
		GLuint shader = glCreateShader(shader_type);

		int shader_source_length = (int)strlen(shader_source);
		glShaderSource(shader, 1, &shader_source, &shader_source_length);
		glCompileShader(shader);

		int info_log_length = 0; glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
		if (info_log_length != 0) {
			std::string info_log; info_log.resize(info_log_length);
			glGetShaderInfoLog(shader, info_log_length, nullptr, const_cast<GLchar*>(info_log.data()));
			printf("%s\n%s\n", shader_type == GL_VERTEX_SHADER ? "vs" : "fs", info_log.c_str());
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
				printf("%s\n", info_log.c_str());
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
				printf("%s\n", info_log.c_str());
			}
		}
		GLint program_validate_status; glGetProgramiv(program, GL_VALIDATE_STATUS, &program_validate_status);
		if (program_validate_status == GL_FALSE) { glDeleteProgram(program); return 0; }

		glUseProgram(program);

		return program;
	}

	void CheckShaderProgram(GLuint program) {
		int info_log_length = 0; glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
		if (!info_log_length) {
			std::string info_log; info_log.resize(info_log_length);
			glGetProgramInfoLog(program, info_log_length, nullptr, const_cast<GLchar*>(info_log.data()));
			printf("%s\n", info_log.c_str());
		}
	}

	void Program::Init(const char* vs_src, const char* fs_src) {
		VS_ID = CreateShader(GL_VERTEX_SHADER, vs_src);
		FS_ID = CreateShader(GL_FRAGMENT_SHADER, fs_src);

		ID = CreateProgram(VS_ID, FS_ID);
	}

	void Program::Release() {
		glDetachShader(ID, VS_ID); glDeleteShader(VS_ID);
		glDetachShader(ID, FS_ID); glDeleteShader(FS_ID);
		glDeleteProgram(ID);
	}

	void Program::Use(bool use) { glUseProgram(use ? ID : 0); }

	void Program::Uniform1i(const char* name, bool value) { glUniform1i(glGetUniformLocation(ID, name), value); }
	void Program::Uniform1i(const char* name, int value) { glUniform1i(glGetUniformLocation(ID, name), value); }
	void Program::Uniform1f(const char* name, float value) { glUniform1f(glGetUniformLocation(ID, name), value); }
	void Program::Uniform3fv(const char* name, smath::float3& value) { glUniform3fv(glGetUniformLocation(ID, name), 1, value.data()); }
	void Program::Uniform3fv(const char* name, float* value) { glUniform3fv(glGetUniformLocation(ID, name), 1, value); }
	void Program::Uniform3f(const char* name, float x, float y, float z) { 
		GLint loc = glGetUniformLocation(ID, name);
		glUniform3f(loc, x, y, z); 
	}
	void Program::Uniform4fv(const char* name, smath::float4& value) { glUniform4fv(glGetUniformLocation(ID, name), 1, value.data()); }
	void Program::Uniform4fv(const char* name, float* value) { glUniform4fv(glGetUniformLocation(ID, name), 1, value); }
	void Program::Uniform4f(const char* name, float x, float y, float z, float w) { glUniform4f(glGetUniformLocation(ID, name), x, y, z, w); }
	void Program::UniformMatrix3fv(const char* name, smath::mat3& value) { glUniformMatrix3fv(glGetUniformLocation(ID, name), 1, GL_TRUE, value.data()); }
	void Program::UniformMatrix3fv(const char* name, float* value) { glUniformMatrix3fv(glGetUniformLocation(ID, name), 1, GL_TRUE, value); }
	void Program::UniformMatrix4fv(const char* name, smath::mat4& value) { glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_TRUE, value.data()); }
	void Program::UniformMatrix4fv(const char* name, float* value) { glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_TRUE, value); }
	
	void Buffer::Init(GLenum target) {
		glGenBuffers(1, &ID);
		this->target = target;

		GLuint buffer_binding = Binding();
		glBindBuffer(target, ID);
		glBindBuffer(target, buffer_binding);
	}
	void Buffer::Release() { glDeleteBuffers(1, &ID); }

	void Buffer::Bind(bool bind) { glBindBuffer(target, bind ? ID : 0); }
	GLuint Buffer::Binding() {
		static std::map<GLenum, GLenum> buffer_bindings = { { GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING },{ GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER_BINDING },{ GL_COPY_READ_BUFFER, GL_COPY_READ_BUFFER_BINDING },{ GL_COPY_WRITE_BUFFER, GL_COPY_WRITE_BUFFER_BINDING },{ GL_PIXEL_UNPACK_BUFFER, GL_PIXEL_UNPACK_BUFFER_BINDING },{ GL_PIXEL_PACK_BUFFER, GL_PIXEL_PACK_BUFFER_BINDING },{ GL_QUERY_BUFFER, GL_QUERY_BUFFER_BINDING },{ GL_TEXTURE_BUFFER, GL_TEXTURE_BUFFER_BINDING },{ GL_TRANSFORM_FEEDBACK_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER_BINDING },{ GL_UNIFORM_BUFFER, GL_UNIFORM_BUFFER_BINDING },{ GL_DRAW_INDIRECT_BUFFER, GL_DRAW_INDIRECT_BUFFER_BINDING },{ GL_ATOMIC_COUNTER_BUFFER, GL_ATOMIC_COUNTER_BUFFER_BINDING },{ GL_DISPATCH_INDIRECT_BUFFER, GL_DISPATCH_INDIRECT_BUFFER_BINDING },{ GL_SHADER_STORAGE_BUFFER, GL_SHADER_STORAGE_BUFFER_BINDING } };
		GLint binding;
		glGetIntegerv(buffer_bindings[target], &binding);
		return binding;
	}

	void Buffer::Storage(GLsizeiptr size, const GLvoid* data, GLbitfield flags) {
		GLuint binding = Binding();
		if (binding != ID) glBindBuffer(target, ID);
		glBufferStorage(target, size, data, flags);
		if (binding != ID) glBindBuffer(target, binding);
	}

	void Buffer::Data(GLsizeiptr size, const GLvoid* data, GLenum usage) {
		GLuint binding = Binding();
		if (binding != ID) glBindBuffer(target, ID);
		glBufferData(target, size, data, usage);
		if (binding != ID) glBindBuffer(target, binding);
	}

	void Buffer::SubData(GLintptr offset, GLsizeiptr size, const GLvoid* data) {
		GLuint binding = Binding();
		if (binding != ID) glBindBuffer(target, ID);
		glBufferSubData(target, offset, size, data);
		if (binding != ID) glBindBuffer(target, binding);
	}

	void Buffer::ClearData(GLenum internalformat, GLenum format, GLenum type, const GLvoid* data) {
		GLuint binding = Binding();
		if (binding != ID) glBindBuffer(target, ID);
		glClearBufferData(target, internalformat, format, type, data);
		if (binding != ID) glBindBuffer(target, binding);
	}

	void Buffer::ClearSubData(GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const GLvoid* data) {
		GLuint binding = Binding();
		if (binding != ID) glBindBuffer(target, ID);
		glClearBufferSubData(target, internalformat, offset, size, format, type, data);
		if (binding != ID) glBindBuffer(target, binding);
	}

	void Buffer::CopySubData(Buffer& readbuffer, Buffer& writebuffer, GLintptr readoffset, GLintptr writeoffset, GLsizeiptr size) {
		GLuint read_binding = readbuffer.Binding();
		readbuffer.Bind();
		GLuint write_binding = writebuffer.Binding();
		writebuffer.Bind();

		glCopyBufferSubData(readbuffer.target, writebuffer.target, readoffset, writeoffset, size);

		glBindBuffer(readbuffer.target, read_binding);
		glBindBuffer(writebuffer.target, write_binding);
	}

	void* Buffer::MapRange(GLintptr offset, GLsizeiptr length, GLbitfield access) {
		binding = Binding();
		if (binding != ID) glBindBuffer(target, ID);
		return glMapBufferRange(target, offset, length, access);
	}

	void* Buffer::Map(GLbitfield access) {
		binding = Binding();
		if (binding != ID) glBindBuffer(target, ID);
		return glMapBuffer(target, access);
	}

	void Buffer::Unmap() {
		glUnmapBuffer(target);
		if (binding != ID) glBindBuffer(target, binding);
	}
	

	
	void VertexBuffer::Init() { target = GL_ARRAY_BUFFER; Buffer::Init(target); }

	void VertexBuffer::Data(size_t count, size_t size, GLvoid* data, GLenum usage) {
		Buffer::Data(count*size, data, usage);
		this->count = count;
	}


	
	void IndexBuffer::Init() { target = GL_ELEMENT_ARRAY_BUFFER; Buffer::Init(target); }

	void IndexBuffer::Data(size_t count, size_t size, GLvoid* data, GLenum usage) {
		Buffer::Data(count*size, data, usage);
		this->count = count;
	}


	void VertexArray::Init() { glGenVertexArrays(1, &ID); }
	void VertexArray::Release() { glDeleteVertexArrays(1, &ID); }

	void VertexArray::Bind(bool bind) { glBindVertexArray(bind ? ID : 0); }
	GLuint VertexArray::Binding() { GLint array_binding; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &array_binding); return array_binding; }

	void VertexArray::AttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer) {
		GLuint binding = Binding();
		if (binding != ID) glBindVertexArray(ID);

		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, type, normalized, stride, pointer);

		if (binding != ID) glBindVertexArray(binding);
	}

	void VertexArray::AttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) {
		GLuint binding = Binding();
		if (binding != ID) glBindVertexArray(ID);

		glEnableVertexAttribArray(index);
		glVertexAttribIPointer(index, size, type, stride, pointer);

		if (binding != ID) glBindVertexArray(binding);
	}

	void VertexArray::AttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) {
		GLuint binding = Binding();
		if (binding != ID) glBindVertexArray(ID);

		glEnableVertexAttribArray(index);
		glVertexAttribLPointer(index, size, type, stride, pointer);

		if (binding != ID) glBindVertexArray(binding);
	}
	
	void Framebuffer::Init(int width, int height) {
		glGenFramebuffers(1, &ID);
		glBindFramebuffer(GL_FRAMEBUFFER, ID);

		// create render texture
		glGenTextures(1, &render_texture);
		glBindTexture(GL_TEXTURE_2D, render_texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// create render buffer
		glGenRenderbuffers(1, &render_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_buffer);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, render_texture, 0);


	}

	void DrawElementsBaseVertex::operator()() {
		glDrawElementsBaseVertex(mode, count, type, indices, basevertex);
	}

}