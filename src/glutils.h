#ifndef CUBE_GLUTILS_H
#define CUBE_GLUTILS_H

#include <GL/glew.h>

enum GLResult {
	Success = 0,
	Error,
	Failed,
	MaxResults,
};

GLResult CompileShader(const char* src, GLenum type, GLuint *shader);
GLResult LinkProgram(GLuint vertex_shader, GLuint fragment_shader, GLuint *program);
void GLAPIENTRY MessageCallback(GLenum source,
				GLenum type,
				GLuint id,
				GLenum severity,
				GLsizei length,
				const GLchar* message,
				const void* userParam );

inline GLResult CheckGLError()
{
	GLResult result = GLResult::Success;

	return result;
}

#endif // CUBE_GLUTILS_H
