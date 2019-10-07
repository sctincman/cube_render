#ifndef CUBE_GLTFSCENE_H
#define CUBE_GLTFSCENE_H

#include <vector>

#include <GL/glew.h>
#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>
#include <tiny_gltf.h>

#include "Scene.h"
#include "Camera.h"
#include "glutils.h"

static const char vs_src[] =
"#version 330\n\
in vec3 position;\n\
in vec3 normal;\n\
in vec4 tangent;\n\
in vec2 texcoord_0;\n\
in vec2 texcoord_1;\n\
in vec4 color_0;\n\
in vec4 joints_0;\n\
in vec4 weights_0;\n\
uniform mat4 world;\n\
out vec2 texcoord;\n\
out vec4 color;\n\
void main() {\n\
    color = color_0;\n\
    texcoord = texcoord_0;\n\
    gl_Position = world * vec4(position, 1.0);\n\
}";

static const char fs_src[] =
"#version 330\n\
in vec4 color;\n\
in vec2 texcoord;\n\
uniform vec4 color_factor;\n\
uniform sampler2D color_texture;\n\
out vec4 out_color;\n\
void main() {\n\
    out_color = color + texture(color_texture, texcoord);\n\
}";


struct GLBufferState
{
	GLuint buffer;
	GLenum target;
};

class GltfScene : public Scene
{
public:
	GltfScene(const char* pFileName);
	~GltfScene();
	void Step(uint32_t stepMs);
	void Render(Camera* pCamera);

private:
	GLResult Init(const char* pFileName);
	void DrawMesh(const tinygltf::Mesh* mesh, glm::mat4 transform);
	void DrawNode(tinygltf::Node* node, glm::mat4 parent_transform);

	tinygltf::Model m_model;
	
	// material?
	GLuint m_program;
	GLuint m_vao;

	// per mesh
	std::vector<GLBufferState> m_glBuffers;
	std::vector<GLuint> m_textures;
};

#endif // CUBE_GLTFSCENE_H
