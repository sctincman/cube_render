#ifndef CUBE_SCENE_H
#define CUBE_SCENE_H

#include <GL/glew.h>
#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>

#include "Camera.h"
#include "glutils.h"

class Scene
{
public:
	Scene();
	~Scene();
	void Step(uint32_t stepMs);
	void Render(Camera *pCamera);

private:
	GLResult Init();
	// TODO general entities?
	GLuint m_program;
	GLuint m_vao, m_vbo, m_ibo;

	float m_t;
	glm::vec3 m_cubePosition;
};

#endif // CUBE_SCENE_H
