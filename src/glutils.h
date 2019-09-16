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

#endif // CUBE_GLUTILS_H
