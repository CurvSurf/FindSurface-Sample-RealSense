#pragma once
#include <map>
#include "smath.h"
#include "3rdparty\glew-2.1.0\include\GL\glew.h"

namespace sgl {
	GLuint CreateShader(GLenum shader_type, const char* shader_source);
	GLuint CreateProgram(GLuint vertex_shader, GLuint fragment_shader);
	void CheckShaderProgram(GLuint program);

	struct Program {
		GLuint ID;
		GLuint VS_ID;
		GLuint FS_ID;

		void Init(const char* vs_src, const char* fs_src);
		void Release();

		void Use(bool use = true);

		void Uniform1i(const char* name, bool value);
		void Uniform1i(const char* name, int value);
		void Uniform1f(const char* name, float value);
		void Uniform3fv(const char* name, smath::float3& value);
		void Uniform3fv(const char* name, float* value);
		void Uniform3f(const char* name, float x, float y, float z);
		void Uniform4fv(const char* name, smath::float4& value);
		void Uniform4fv(const char* name, float* value);
		void Uniform4f(const char* name, float x, float y, float z, float w);
		void UniformMatrix3fv(const char* name, smath::mat3& value);
		void UniformMatrix3fv(const char* name, float* value);
		void UniformMatrix4fv(const char* name, smath::mat4& value);
		void UniformMatrix4fv(const char* name, float* value);
	};

	struct Buffer {
		GLuint ID;
		GLenum target;
		GLuint binding;

		void Init(GLenum target);
		void Release();

		void Bind(bool bind = true);
		GLuint Binding();

		void Storage(GLsizeiptr size, const GLvoid* data, GLbitfield flags);
		void Data(GLsizeiptr size, const GLvoid* data, GLenum usage);
		void SubData(GLintptr offset, GLsizeiptr size, const GLvoid* data);
		void ClearData(GLenum internalformat, GLenum format, GLenum type, const GLvoid* data);
		void ClearSubData(GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const GLvoid* data);
		static void CopySubData(Buffer& readbuffer, Buffer& writebuffer, GLintptr readoffset, GLintptr writeoffset, GLsizeiptr size);
		void* MapRange(GLintptr offset, GLsizeiptr length, GLbitfield access);
		void* Map(GLbitfield access);
		void Unmap();
	};

	struct VertexBuffer : Buffer {
		GLsizei count=0;

		void Init();

		void Data(size_t count, size_t size, GLvoid* data, GLenum usage);
	};

	struct IndexBuffer : Buffer {
		GLsizei count=0;

		void Init();

		void Data(size_t count, size_t size, GLvoid* data, GLenum usage);
	};

	struct VertexArray {
		GLuint ID;

		void Init();
		void Release();

		void Bind(bool bind = true);
		GLuint Binding();

		void AttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
		void AttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
		void AttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
	};

	struct DrawArrays {
		GLenum mode; 
		GLint first;
		GLsizei count;

		DrawArrays() : mode(GL_POINTS), first(), count() {}
		DrawArrays(GLenum mode, GLint first, GLsizei count) : mode(mode), first(first), count(count) {}

		void operator()();
	};

	struct DrawElementsBaseVertex {
		GLenum mode = GL_POINTS; 
		GLsizei count = 0;
		GLenum type = GL_UNSIGNED_INT;
		GLvoid* indices = 0;
		GLint basevertex = 0;

		DrawElementsBaseVertex() : mode(GL_POINTS), count(), type(GL_UNSIGNED_INT), indices(), basevertex() {}
		DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, GLvoid* indices, GLint basevertex) : mode(mode), count(count), type(type), indices(indices), basevertex(basevertex) {}

		void operator()();
	};

}