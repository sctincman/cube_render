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

	for (size_t i = 0; i < m_textures.size(); ++i)
	{
		glDeleteTextures(1, &m_textures[i]);
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

	if (result == GLResult::Success)
	{
		m_textures.reserve(m_model.textures.size());

		for (size_t i = 0; i < m_model.textures.size(); ++i)
		{
			tinygltf::Texture* texture = &m_model.textures[i];
			GLuint* textureId = &m_textures[i];

			glGenTextures(1, textureId);
			glBindTexture(GL_TEXTURE_2D, *textureId);

			tinygltf::Image* image = &m_model.images[texture->source];

			GLenum format = GL_RGBA;
			if (image->component == 3) {
				format = GL_RGB;
			}

			glTexImage2D(GL_TEXTURE_2D,
				     0,
				     format,
				     image->width,
				     image->height,
				     0,
				     format,
				     image->pixel_type,
				     &image->image.at(0));


			if (texture->sampler < 0)
			{
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			} else {
				tinygltf::Sampler* sampler = &m_model.samplers[texture->sampler];
				glTexParameterf(GL_TEXTURE_2D,
						GL_TEXTURE_MIN_FILTER,
						sampler->minFilter);
				glTexParameterf(GL_TEXTURE_2D,
						GL_TEXTURE_MAG_FILTER,
						sampler->magFilter);
				glTexParameterf(GL_TEXTURE_2D,
						GL_TEXTURE_WRAP_S,
						sampler->wrapS);
				glTexParameterf(GL_TEXTURE_2D,
						GL_TEXTURE_WRAP_T,
						sampler->wrapT);

				if ((sampler->minFilter == GL_NEAREST_MIPMAP_NEAREST) ||
				    (sampler->minFilter == GL_NEAREST_MIPMAP_LINEAR) ||
				    (sampler->minFilter == GL_LINEAR_MIPMAP_NEAREST) ||
				    (sampler->minFilter == GL_LINEAR_MIPMAP_LINEAR))
				{
					glGenerateMipmap(GL_TEXTURE_2D);		
				}
			}

			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	if (result == GLResult::Success)
	{
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
	}
	
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

	GLint color_factor_uniform = glGetUniformLocation(m_program, "color_factor");
	if (world_uniform == -1) {
		fprintf(stderr, "[ERROR] could not get uniform location\n");
	}

	GLint color_tex_uniform = glGetUniformLocation(m_program, "color_texture");
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
		if (primitive->material >= 0)
		{
			tinygltf::Material* material = &m_model.materials[primitive->material];

			/* Yay PBR? */
			if (material->values.find("baseColorFactor") != material->values.end())
			{
			} else {
				glUniform4f(color_factor_uniform,
					    0., 0., 0., 0.);
			}

			if (material->values.find("baseColorTexture") != material->values.end())
			{
				tinygltf::Parameter* texParam = &material->values["baseColorTexture"];

				int texIdx = texParam->TextureIndex();
				if (texIdx >= 0 ||
				    texIdx < m_textures.size())
				{
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, m_textures[texIdx]);
					glUniform1i(color_tex_uniform,
						    1);
				} else {
					glUniform1i(color_tex_uniform, 0);
				}

				//todo tex coord match?
					
			} else {
				glUniform1i(color_tex_uniform,
					    0);
			}

		}
		
		// handle attributes
		for (std::map<std::string, int>::const_iterator it = primitive->attributes.begin(); it != primitive->attributes.end(); ++it)
		{
		        char* attributeName = strlwr(it->first.c_str());

			GLint location = glGetAttribLocation(m_program, attributeName);
			if (location == -1) {
				//fprintf(stderr, "[WARN] draw has unused attribute: %s\n", attributeName);
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

void GltfScene::DrawNode(tinygltf::Node* node, glm::mat4 parent_transform)
{
	glm::mat4 local_transform = glm::mat4(1.0f);
	if (node->matrix.size() == 16)
	{
		local_transform = glm::make_mat4(node->matrix.data());
	}
	else
	{
		if (node->scale.size() == 3)
			local_transform = glm::scale(glm::vec3(
							     node->scale[0],
							     node->scale[1],
							     node->scale[2]))
				* local_transform;
		if (node->rotation.size() == 4)
			local_transform = glm::mat4_cast(glm::quat(node->rotation[3],
								   node->rotation[0],
								   node->rotation[1],
								   node->rotation[2]))
				* local_transform;
		if (node->translation.size() == 3)
			local_transform = glm::translate(
				glm::vec3(
					node->translation[0],
					node->translation[1],
					node->translation[2])) * local_transform;
	}

	local_transform = parent_transform * local_transform;

	if (node->mesh >= 0)
	{
		tinygltf::Mesh* mesh = &m_model.meshes[node->mesh];
		DrawMesh(mesh, local_transform);
	}

	for (size_t i = 0; i < node->children.size(); ++i)
	{
		int nodeId = node->children[i];
		if (nodeId < m_model.nodes.size())
		{
			tinygltf::Node* node = &m_model.nodes[nodeId];
			DrawNode(node, local_transform);
		}
		else
		{
			fprintf(stderr, "[ERROR] Child ID out of bound %u [bounds %u]",
				nodeId, m_model.nodes.size());
		}
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

	unsigned int scene = (m_model.defaultScene < 0) ? 0 : m_model.defaultScene;

	if (m_model.scenes.size() >= scene)
	{
		for (size_t i = 0; i < m_model.scenes[scene].nodes.size(); ++i)
		{
			int nodeId = m_model.scenes[scene].nodes[i];
			DrawNode(&m_model.nodes[nodeId], view_project);
		}
	}
}
