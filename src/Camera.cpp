#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/rotate_vector.hpp>

Camera::Camera(float width, float height, float fov, float znear, float zfar, float scale) {
	
	position = glm::vec3(0.0f);
	targeting = true;
	target = glm::vec3(0.0f, 1.5f, -5.0f);
	direction = glm::vec3(0.0f, 0.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);

	this->fov = fov;
	this->znear = znear;
	this->zfar = zfar;
	this->scale = scale;
	perspective = glm::perspective(fov, width/height, znear, zfar);
	orthographic = glm::ortho(-(width * scale) / height,
				   (width * scale) / height,
				   -scale,
				   scale,
				   znear,
				   zfar);
	
	vertical = MoveVertical::Not;
	horizontal = MoveHorizontal::Not;
	rotate = RotateZ::Not;
}

glm::mat4 Camera::View() {
	///@todo much easier with ECS? can hold ref to target and query position
	if (targeting) {
		return glm::lookAt(position, target, up);
	} else {
		glm::vec3 direction_target = position + direction;
		return glm::lookAt(position, direction_target, up);
	}
}

glm::mat4 Camera::Projection(bool perspective) {
	if (perspective)
		return this->perspective;
	else
		return this->orthographic;
}

bool Camera::HandleKeys(SDL_KeyboardEvent event) {
	bool result = false;
	//TODO fsm
	//TODO change to a configurable LUT?
	switch (event.keysym.sym) {
	//Lock on/off toggle
	case SDLK_TAB:
		if (event.state == SDL_PRESSED) {
			targeting = !targeting;
			result = true;
		}
		break;

	case SDLK_w:
		if (event.state == SDL_PRESSED) {
			if (vertical == MoveVertical::Not) {
				vertical = MoveVertical::Up;
				result = true;
			}
		} else {
			if (vertical == MoveVertical::Up) {
				vertical = MoveVertical::Not;
				result = true;
			}
		}
		break;

	case SDLK_s:
		if (event.state == SDL_PRESSED) {
			if (vertical == MoveVertical::Not) {
				vertical = MoveVertical::Down;
				result = true;
			}
		} else {
			if (vertical == MoveVertical::Down) {
				vertical = MoveVertical::Not;
				result = true;
			}
		}
		break;

	case SDLK_q:
		if (event.state == SDL_PRESSED) {
			if (horizontal == MoveHorizontal::Not) {
				horizontal = MoveHorizontal::Left;
				result = true;
			}
		} else {
			if (horizontal == MoveHorizontal::Left) {
				horizontal = MoveHorizontal::Not;
				result = true;
			}
		}
		break;

	case SDLK_e:
		if (event.state == SDL_PRESSED) {
			if (horizontal == MoveHorizontal::Not) {
				horizontal = MoveHorizontal::Right;
				result = true;
			}
		} else {
			if (horizontal == MoveHorizontal::Right) {
				horizontal = MoveHorizontal::Not;
				result = true;
			}
		}
		break;

	case SDLK_a:
		if (event.state == SDL_PRESSED) {
			if (rotate == RotateZ::Not) {
				rotate = RotateZ::Left;
				result = true;
			}
		} else {
			if (rotate == RotateZ::Left) {
				rotate = RotateZ::Not;
				result = true;
			}
		}
		break;

	case SDLK_d:
		if (event.state == SDL_PRESSED) {
			if (rotate == RotateZ::Not) {
				rotate = RotateZ::Right;
				result = true;
			}
		} else {
			if (rotate == RotateZ::Right) {
				rotate = RotateZ::Not;
				result = true;
			}
		}
		break;
	default:
		result = false;
		break;
	}

	return result;
}

bool Camera::HandleMouseMotion(SDL_MouseMotionEvent event)
{
	float delta_x = event.xrel / this->width;
	float delta_y = event.yrel / this->height;
	Rotate(delta_x, delta_y);

	return true;
}

bool Camera::HandleInputEvent(SDL_Event event)
{
	bool result = false;
	switch(event.type) {
	case SDL_MOUSEMOTION:
		result = HandleMouseMotion(event.motion);
		break;
	case SDL_KEYUP:
	case SDL_KEYDOWN:
		result = HandleKeys(event.key);
		break;
	}
	return result;
}

void Camera::Rotate(float delta_x, float delta_y) {
	if (!targeting) {
		direction = glm::rotate(direction,
					delta_x * fov,
					up);
		direction = glm::rotate(direction,
					delta_y * fov,
					glm::cross(direction, up));
	}
}

void Camera::Resize(float width, float height) {
	this->width = width;
	this->height = height;
	perspective = glm::perspective(fov, width/height, znear, zfar);
	orthographic = glm::ortho(-(width * scale) / height,
				   (width * scale) / height,
				  -scale,
				   scale,
				   znear,
				   zfar);
}

glm::vec3 Camera::Move(glm::vec3 delta)
{
	position += delta;
	return position;
}

void Camera::SetPosition(glm::vec3 position)
{
	this->position = position;
}

void Camera::Target(glm::vec3 target, glm::vec3 new_up)
{
	this->target = target;
	this->up = new_up;
	this->direction = target - position;
}

void Camera::Step(long delta=16) {
	if (targeting) {
		direction = glm::fastNormalize(target - position);
	}

	switch (vertical) {
	case MoveVertical::Up:
		position += direction * (delta * 0.01f);
		break;
	case MoveVertical::Down:
		position -= direction * (delta * 0.01f);
		break;
	}

	switch (horizontal) {
	case MoveHorizontal::Left:
		position -= glm::cross(direction, up) * (delta * 0.01f);
		break;
	case MoveHorizontal::Right:
		position += glm::cross(direction, up) * (delta * 0.01f);
		break;
	}

	switch (rotate) {
	case RotateZ::Left:
		Rotate(0.004 * delta, 0.0);
		break;
	case RotateZ::Right:
		Rotate(-0.004 * delta, 0.0);
		break;
	}
}

