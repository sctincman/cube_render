#include "Scene.h"

#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

static GLfloat vertices[] = {-0.5, -0.5, -0.5,
			     0.5, -0.5, -0.5,
			     -0.5, -0.5,  0.5,
			     0.5, -0.5,  0.5,
			     
			     -0.5, -0.5,  0.5,
			     0.5, -0.5,  0.5,
			     -0.5,  0.5,  0.5,
			     0.5,  0.5,  0.5,
			     
			     -0.5, -0.5, -0.5,
			     -0.5, -0.5,  0.5,
			     -0.5,  0.5, -0.5,
			     -0.5,  0.5,  0.5,
			     
			     0.5, -0.5, -0.5,
			     0.5, -0.5,  0.5,
			     0.5,  0.5, -0.5,
			     0.5,  0.5,  0.5,
			     
			     -0.5, -0.5, -0.5,
			     0.5, -0.5, -0.5,
			     -0.5,  0.5, -0.5,
			     0.5,  0.5, -0.5,
                             
			     -0.5,  0.5, -0.5,
			     0.5,  0.5, -0.5,
			     -0.5,  0.5,  0.5,
			     0.5,  0.5,  0.5};

static GLuint indices[] = {0, 1, 2,
			   1, 2, 3,

			   4, 5, 6,
			   5, 6, 7,

			   8, 9, 10,
			   9, 10, 11,

			   12, 13, 14,
			   13, 14, 15,

			   16, 17, 18,
			   17, 18, 19,

			   20, 21, 22,
			   21, 22, 23};

static const char vs_src[] =
"#version 330\n\
in vec3 position;\n\
out vec4 color;\n\
uniform mat4 world;\n\
void main() {\n\
    color = vec4(clamp(position + 0.5f, 0.0, 1.0), 1.0);\n\
    gl_Position = world * vec4(position, 1.0);\n\
}";

static const char fs_src[] =
"#version 330\n\
in  vec4 color;\n\
out vec4 out_color;\n\
void main() {\n\
    out_color = color;\n\
}";

Scene::Scene()
{
	Init();
}

Scene::~Scene()
{
	glDeleteProgram(m_program);
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ibo);
	glDeleteVertexArrays(1, &m_vao);
}

GLResult Scene::Init()
{
	GLResult result = GLResult::Success;
	GLuint vs, fs;

	result = CompileShader(vs_src, GL_VERTEX_SHADER, &vs);
	result = CompileShader(fs_src, GL_FRAGMENT_SHADER, &fs);
	result = LinkProgram(vs, fs, &m_program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	glBindFragDataLocation(m_program,
			       0,
			       "out_color");
	
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER,
		     sizeof(vertices),
		     vertices,
		     GL_STATIC_DRAW);

	GLint pos_attr = glGetAttribLocation(m_program, "position");
	if (pos_attr == -1) {
		return GLResult::Error;
	}
	glEnableVertexAttribArray(pos_attr);
	glVertexAttribPointer(pos_attr,
			      3,
			      GL_FLOAT,
			      GL_FALSE,
			      0,
			      nullptr);

	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		     sizeof(indices),
		     indices,
		     GL_STATIC_DRAW);

	return result;
}

void Scene::Step(uint32_t stepMs)
{
	m_t += static_cast<float>(stepMs) / 400.0;
        m_cubePosition = glm::vec3(sin(m_t), sin(m_t*2.0) + 1.5f, -5.0f);
}

void Scene::Render(Camera *pCamera)
{
	GLint world_uniform = glGetUniformLocation(m_program, "world");
	if (world_uniform == -1) {
		fprintf(stderr, "[ERROR] could not get uniform location");
	}

	glClearColor(0.2, 0.2, 0.2, 0.2);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	glm::mat4 model = glm::translate(m_cubePosition);
	glm::mat4 view = pCamera->View();
	glm::mat4 model_view_projection = pCamera->Projection(false) * view * model;

	glClear(GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_program);
	glUniformMatrix4fv(world_uniform,
			   1,
			   GL_FALSE,
			   glm::value_ptr(model_view_projection));

	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES,
		       36,
		       GL_UNSIGNED_INT,
		       nullptr);

}
