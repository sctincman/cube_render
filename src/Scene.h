#ifndef CUBE_SCENE_H
#define CUBE_SCENE_H

#include "Camera.h"

class Scene
{
public:
	virtual ~Scene() {};
	virtual void Step(uint32_t stepMs) = 0;
	virtual void Render(Camera *pCamera) = 0;
};

#endif // CUBE_SCENE_H
