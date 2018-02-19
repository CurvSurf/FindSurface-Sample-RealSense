#pragma once
#include "smath.h"
#include "opengl_wrapper.h"

struct Renderer {
	sgl::Program program;
	sgl::VertexArray vertex_array;
};

struct GeometryRenderer : Renderer {
	sgl::VertexBuffer position_buffer;
	sgl::IndexBuffer index_buffer;

	smath::mat4 view_matrix;
	smath::mat4 projection_matrix;
};

struct PlaneRenderer : GeometryRenderer {
	sgl::DrawArrays draw;

	float* quad[4] = { nullptr, };
	
	void render() {
		vertex_array.Bind();
		program.Use();
		program.Uniform3f("color", 1, 0, 0);
		program.Uniform3fv("quad[0]", quad[0]);
		program.Uniform3fv("quad[1]", quad[1]);
		program.Uniform3fv("quad[2]", quad[2]);
		program.Uniform3fv("quad[3]", quad[0]);
		program.Uniform3fv("quad[4]", quad[2]);
		program.Uniform3fv("quad[5]", quad[3]);
		program.UniformMatrix4fv("view_matrix", view_matrix);
		program.UniformMatrix4fv("projection_matrix", projection_matrix);

		draw();

		program.Use(false);
		vertex_array.Bind(false);
	}
};

struct SphereRenderer : GeometryRenderer {
	sgl::DrawElementsBaseVertex draw;

	smath::mat4 model_matrix;
	void render() {
		vertex_array.Bind();
		program.Use();
		
		program.Uniform3f("color", 1, 0, 0);
		program.UniformMatrix4fv("model_matrix", model_matrix);
		program.UniformMatrix4fv("view_matrix", view_matrix);
		program.UniformMatrix4fv("projection_matrix", projection_matrix);

		draw();

		program.Use(false);
		vertex_array.Bind(false);
	}
};

struct CylinderRenderer : GeometryRenderer {
	sgl::DrawElementsBaseVertex draw;

	smath::mat4 model_matrix;
	void render() {
		vertex_array.Bind();
		program.Use();

		program.Uniform3f("color", 1, 0, 0);
		program.UniformMatrix4fv("model_matrix", model_matrix);
		program.UniformMatrix4fv("view_matrix", view_matrix);
		program.UniformMatrix4fv("projection_matrix", projection_matrix);

		draw();

		program.Use(false);
		vertex_array.Bind(false);
	}
};

struct ConeRenderer : GeometryRenderer {
	sgl::DrawElementsBaseVertex draw;

	float top_radius;
	float bottom_radius;
	smath::mat4 model_matrix;
	void render() {
		vertex_array.Bind();
		program.Use();

		program.Uniform3f("color", 1, 0, 0);
		program.Uniform1f("top_radius", top_radius);
		program.Uniform1f("bottom_radius", bottom_radius);
		program.UniformMatrix4fv("model_matrix", model_matrix);
		program.UniformMatrix4fv("view_matrix", view_matrix);
		program.UniformMatrix4fv("projection_matrix", projection_matrix);

		draw();

		program.Use(false);
		vertex_array.Bind(false);
	}
};

struct TorusRenderer : GeometryRenderer {
	sgl::DrawElementsBaseVertex draw;

	float angle;
	float mean_radius;
	float tube_radius;
	smath::mat4 model_matrix;
	void render() {
		vertex_array.Bind();
		program.Use();

		program.Uniform3f("color", 1, 0, 0);
		program.Uniform1f("mean_radius", mean_radius);
		program.Uniform1f("tube_radius", tube_radius);
		program.UniformMatrix4fv("model_matrix", model_matrix);
		program.UniformMatrix4fv("view_matrix", view_matrix);
		program.UniformMatrix4fv("projection_matrix", projection_matrix);

		int count = int(std::ceil((draw.count / 60)*angle / (2.f*smath::PI))) * 60;
		glDrawElementsBaseVertex(draw.mode, count, draw.type, draw.indices, draw.basevertex);

		program.Use(false);
		vertex_array.Bind(false);
	}
};

struct PointCloudRenderer : Renderer {
	sgl::VertexBuffer position_buffer;
	sgl::VertexBuffer color_buffer;

	smath::mat4 view_matrix;
	smath::mat4 projection_matrix;

	sgl::DrawArrays draw;

	void render() {
		vertex_array.Bind();
		program.Use();

		program.UniformMatrix4fv("view_matrix", view_matrix);
		program.UniformMatrix4fv("projection_matrix", projection_matrix);

		glDrawArrays(draw.mode, 0, position_buffer.count);

		program.Use(false);
		vertex_array.Bind(false);
	}
};

struct ImageRenderer : Renderer {
	GLuint texture = 0;
	sgl::Buffer PBO[2];

	sgl::DrawArrays draw;

	void render(int width, int height, uint8_t* color_image) {
		// 1. PBO ping pong
		static int index = 0;
		int next_index = 0;

		index = (index + 1) % 2;
		next_index = (index + 1) % 2;

		// 2. color image data captured from Intel RealSense device will be transferred from PBO to texture
		// The data was sent to PBO by the code below when the previous frame is rendered.
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		PBO[index].Bind();

		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		// 3. data transfer from memory (CPU) to PBO (GPU)
		// Now we send new data to PBO, and it will be transferred to the texture when the next frame is rendered.
		GLsizeiptr data_size = width*height * 3;
		PBO[next_index].Data(data_size, nullptr, GL_STREAM_DRAW);

		GLubyte*ptr = (GLubyte*)PBO[next_index].Map(GL_WRITE_ONLY);
		if (ptr) {
			memcpy(ptr, color_image, data_size);
			PBO[next_index].Unmap();
		}

		// 4. render the color image to screen.
		program.Use(true);
		program.Uniform1i("color_texture", 0);
		vertex_array.Bind(true);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		vertex_array.Bind(false);
		program.Use(false);
	}
};