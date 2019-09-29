#include "glutils.h"
#include "log.h"

GLResult CompileShader(const char* src, GLenum type, GLuint *shader) {
	GLResult result = GLResult::Success;
	GLuint new_shader = glCreateShader(type);

	if (new_shader == 0) {
		result = GLResult::Error;
	}

	if (result == GLResult::Success) {
		glShaderSource(new_shader, 1, &src, nullptr);
		glCompileShader(new_shader);

		GLint status = GL_FALSE;
		glGetShaderiv(new_shader, GL_COMPILE_STATUS, &status);

		if (status != GL_TRUE) {
			result = GLResult::Failed;
			GLint len = 0;
			glGetShaderiv(new_shader, GL_INFO_LOG_LENGTH, &len);
			char *log = new char[len];
			glGetShaderInfoLog(new_shader, len, nullptr, log);
			fprintf(stderr, "[ERROR] Shader compilation failed: %s", log);
			delete[] log;
			glDeleteShader(new_shader);
		}
	}

	if (result == GLResult::Success) {
		*shader = new_shader;
	}

	return result;
}

GLResult LinkProgram(GLuint vertex_shader, GLuint fragment_shader, GLuint *program) {
	GLResult result = GLResult::Success;
	GLuint new_program = glCreateProgram();

	if (new_program == 0) {
		result = GLResult::Error;
	}

	if (result == GLResult::Success) {
		glAttachShader(new_program, vertex_shader);
		glAttachShader(new_program, fragment_shader);
		glLinkProgram(new_program);

		GLint status = GL_FALSE;
		glGetProgramiv(new_program, GL_LINK_STATUS, &status);

		if (status != GL_TRUE) {
			result = GLResult::Failed;
			GLint len = 0;
			glGetProgramiv(new_program, GL_INFO_LOG_LENGTH, &len);
			char *log = new char[len];
			glGetProgramInfoLog(new_program, len, nullptr, log);
			fprintf(stderr, "[ERROR] Program linking failed: %s", log);
			delete[] log;
			glDeleteProgram(new_program);
		}
	}

	if (result == GLResult::Success) {
		*program = new_program;
	}

	return result;
}

void GLAPIENTRY MessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam )
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
		type, severity, message );
}
