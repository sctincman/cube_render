#ifndef CUBE_TESTSCENE_H
#define CUBE_TESTSCENE_H

#include <GL/glew.h>
#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>

#include "Scene.h"
#include "Camera.h"
#include "glutils.h"

class TestScene : public Scene
{
public:
	TestScene();
	~TestScene();
	void Step(uint32_t stepMs);
	void Render(Camera *pCamera);

private:
	GLResult Init();

	GLuint m_program;
	GLuint m_vao, m_vbo, m_ibo;

	float m_t;
	glm::vec3 m_cubePosition;
};

#endif // CUBE_TESTSCENE_H
