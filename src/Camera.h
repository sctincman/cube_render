#ifndef CUBE_CAMERA_H
#define CUBE_CAMERA_H

#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>

#include <SDL2/SDL.h>

enum class MoveVertical {
	Not = 0,
	Up,
	Down
};

enum class MoveHorizontal {
	Not = 0,
	Left,
	Right,
};

enum class RotateZ {
	Not = 0,
	Left,
	Right,
};

class Camera {
private:
	glm::vec3 position;
	bool targeting;
	bool persp;
	glm::vec3 target;
	glm::vec3 direction;
	glm::vec3 up;
	float width, height;
	float fov;
	float znear;
	float zfar;
	glm::mat4 perspective;
	glm::mat4 orthographic;
	float scale;

	bool HandleKeys(SDL_KeyboardEvent event);
	bool HandleMouseMotion(SDL_MouseMotionEvent event);

	MoveVertical vertical;
	MoveHorizontal horizontal;
	RotateZ rotate;
public:
	Camera(float width, float height, float fov, float znear, float zfar, float scale);
	glm::mat4 View();
	glm::mat4 Projection();
	void SetPerspective(bool persp);
	void Rotate(float delta_x, float delta_y);
	void Resize(float width, float height);
	void Reproject();
	glm::vec3 GetPosition() { return position;}
	glm::vec3 Move(glm::vec3 delta);
	void SetPosition(glm::vec3 position);
	void Target(glm::vec3 target);
	void Target(glm::vec3 target, glm::vec3 new_up);

	bool HandleInputEvent(SDL_Event event);
	void Step(long delta);
};

#endif //CUBE_CAMERA_H
