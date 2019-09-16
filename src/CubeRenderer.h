#ifndef CUBE_RENDERER_H
#define CUBE_RENDERER_H

#include <GL/glew.h>
#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include "Camera.h"
#include "Scene.h"
#include "glutils.h"

#define NUM_SIDES 6

class CubeRenderer
{
public:
	CubeRenderer(uint32_t width, uint32_t height);
	~CubeRenderer();
	void Resize(uint32_t width, uint32_t height);
	void Step(uint32_t stepMs);
	void Render(Scene *pTargetScene);

	bool HandleInputEvent(SDL_Event event);

private:
	GLResult Init();
       
	// TODO general entities?
	GLuint m_program;
	GLuint m_vao, m_vbos[2], m_ibo;
	GLuint m_fbos[NUM_SIDES];
	GLuint m_color;
	GLuint m_depth;

	uint32_t m_width, m_height;
	bool persp;
	float m_t;
	glm::vec3 m_centerPosition;
	float m_extent;

	Camera *pCubeCamera;
	Camera *pAppCamera;
};

#endif // CUBE_RENDERER_H
