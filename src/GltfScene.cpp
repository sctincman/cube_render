#include "GltfScene.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>


GltfScene::GltfScene(const char* pFileName)
{
	Init(pFileName);
}

GltfScene::~GltfScene()
{
	glDeleteProgram(m_program);

	glDeleteVertexArrays(1, &m_vao);

	for (size_t i = 0; i < m_glBuffers.size(); ++i)
	{
		GLBufferState* state = &m_glBuffers[i];
		glDeleteBuffers(1, &state->buffer);
	}
}

GLResult GltfScene::Init(const char* pFileName)
{
	GLResult result = GLResult::Success;

	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;
	bool ret = true;

	// detect gltf vs gltfb?
	ret = loader.LoadASCIIFromFile(&m_model, &err, &warn, pFileName);

	if (ret == false)
	{
		result = GLResult::Failed;
		if (err.empty() == false)
		{
			fprintf(stderr, "[ERROR] Could not load GLTF from file %s: %s\n",
				pFileName, err.c_str());
		}
	}

	if (warn.empty() == false)
	{
		fprintf(stderr, "[WARN] GLTF loading file %s: %s\n",
			pFileName, warn.c_str());
	}

	if (result == GLResult::Success)
	{
		m_glBuffers.reserve(m_model.bufferViews.size());

		for (size_t i = 0; i < m_model.bufferViews.size(); ++i)
		{
			GLBufferState* state = &m_glBuffers[i];
			const tinygltf::BufferView* bufferView = &m_model.bufferViews[i];
			const tinygltf::Buffer*     buffer = &m_model.buffers[bufferView->buffer];
			if (bufferView->target == 0) {
				fprintf(stderr, "[WARN] bufferView target is %x is not supported", bufferView->target);
				continue;
			}

			state->target = bufferView->target;

			glGenBuffers(1, &state->buffer);
			glBindBuffer(state->target, state->buffer);
			glBufferData(state->target,
				     bufferView->byteLength,
				     &buffer->data[bufferView->byteOffset],
				     GL_STATIC_DRAW);
			glBindBuffer(state->target, 0);
		}
	}

	glGenVertexArrays(1, &m_vao);

	//when rendering, iterate nodes and follow references to meshes and such
	GLuint vs, fs;

	result = CompileShader(vs_src, GL_VERTEX_SHADER, &vs);
	result = CompileShader(fs_src, GL_FRAGMENT_SHADER, &fs);
	result = LinkProgram(vs, fs, &m_program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	glBindFragDataLocation(m_program,
			       0,
			       "out_color");
	
	return result;
}

void GltfScene::Step(uint32_t stepMs)
{

}

inline char* strlwr(const char* c_string)
{
	char* lowerString = reinterpret_cast<char*>(malloc(strlen(c_string)));

	char* p = lowerString;

	while (*c_string != '\0')
	{
		*p = tolower(*c_string);
		++p;
		++c_string;
	}

	return lowerString;
}

void GltfScene::DrawMesh(const tinygltf::Mesh* mesh, glm::mat4 transform)
{
	GLint world_uniform = glGetUniformLocation(m_program, "world");
	if (world_uniform == -1) {
		fprintf(stderr, "[ERROR] could not get uniform location\n");
	}

	glUniformMatrix4fv(world_uniform,
			   1,
			   GL_FALSE,
			   glm::value_ptr(transform));

	// loop primitives, assembling draws
	for (size_t i = 0; i < mesh->primitives.size(); ++i)
	{
		glBindVertexArray(m_vao);
		const tinygltf::Primitive* primitive = &mesh->primitives[i];
		// handle material
		// glUseProgram?
		// uniforms, textures, samplers?
		
		// handle attributes
		for (std::map<std::string, int>::const_iterator it = primitive->attributes.begin(); it != primitive->attributes.end(); ++it)
		{
		        char* attributeName = strlwr(it->first.c_str());

			GLint location = glGetAttribLocation(m_program, attributeName);
			if (location == -1) {
				fprintf(stderr, "[WARN] draw has unused attribute: %s\n", attributeName);
				free(attributeName);
				continue;
			}
			free(attributeName);

			tinygltf::Accessor* accessor = &m_model.accessors[it->second];
			tinygltf::BufferView* bufferView = &m_model.bufferViews[accessor->bufferView];
			GLBufferState* state = &m_glBuffers[accessor->bufferView];

			if (state->target != GL_ARRAY_BUFFER)
				continue;

			glBindBuffer(GL_ARRAY_BUFFER, state->buffer);
			
			glEnableVertexAttribArray(location);
			glVertexAttribPointer(location,
					      tinygltf::GetTypeSizeInBytes(static_cast<uint32_t>(accessor->type)),
					      accessor->componentType,
					      accessor->normalized ? GL_TRUE : GL_FALSE,
					      bufferView->byteStride,
					      nullptr);

		}
		
		if (primitive->indices < 0)
		{
			tinygltf::Accessor* accessor = &m_model.accessors[primitive->attributes.begin()->second];
			// accessor would already have offset t start
			glDrawArrays(primitive->mode, 0, accessor->count);
		}
		else
		{
			tinygltf::Accessor* accessor = &m_model.accessors[primitive->indices];
			GLBufferState* state = &m_glBuffers[accessor->bufferView];
			if (state->target != GL_ELEMENT_ARRAY_BUFFER)
				fprintf(stderr, "[WARN] Buffer used for indicies that isn't marked as element array buffer target\n");
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->buffer);
			GLenum mode = (primitive->mode < 0) ? GL_TRIANGLES : primitive->mode;
			glDrawElements(mode,
				       accessor->count,
				       accessor->componentType,
				       static_cast<void*>(NULL) + accessor->byteOffset);
		}
		glBindVertexArray(0);
	}
}

void GltfScene::Render(Camera* pCamera)
{
	glClearColor(0.2, 0.2, 0.2, 0.2);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	glClear(GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT);


	glm::mat4 view_project = pCamera->Projection() * pCamera->View();

	glUseProgram(m_program);

	for (size_t i = 0; i < m_model.meshes.size(); ++i)
	{
		tinygltf::Mesh* mesh = &m_model.meshes[i];
		glm::mat4 model = glm::translate(glm::vec3(0.0f, 0.0f, i * -5.0f));
		DrawMesh(mesh, view_project * model);
	}
}
