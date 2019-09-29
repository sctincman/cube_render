#include "CubeRenderer.h"

#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

static GLfloat vertices[] = {-0.5, -0.5, -0.5,
			      0.5, -0.5, -0.5,
			     -0.5, -0.5,  0.5,
			      0.5, -0.5,  0.5,
			     
			     -0.5, -0.5,  0.5,
			      0.5, -0.5,  0.5,
			     -0.5,  0.5,  0.5,
			      0.5,  0.5,  0.5,
			     
			     -0.5, -0.5, -0.5,
			     -0.5, -0.5,  0.5,
			     -0.5,  0.5, -0.5,
			     -0.5,  0.5,  0.5,
			     
			      0.5, -0.5, -0.5,
			      0.5, -0.5,  0.5,
			      0.5,  0.5, -0.5,
			      0.5,  0.5,  0.5,
			     
			     -0.5, -0.5, -0.5,
			      0.5, -0.5, -0.5,
			     -0.5,  0.5, -0.5,
			      0.5,  0.5, -0.5,
                             
			     -0.5,  0.5, -0.5,
			      0.5,  0.5, -0.5,
			     -0.5,  0.5,  0.5,
			      0.5,  0.5,  0.5};

static GLfloat uvs[] = { -1.0,  -1.0,  1.0,
			  1.0,  -1.0,  1.0,
			 -1.0,  -1.0, -1.0,
			  1.0,  -1.0, -1.0,
			     
			 -1.0,   1.0,  1.0,
			  1.0,   1.0,  1.0,
			 -1.0,  -1.0,  1.0,
			  1.0,  -1.0,  1.0,

			  1.0,  1.0,   1.0,
			  1.0,  1.0,  -1.0,
			  1.0, -1.0,   1.0,
			  1.0, -1.0,  -1.0,

			 -1.0,  1.0,   1.0,
			 -1.0,  1.0,  -1.0,
			 -1.0, -1.0,   1.0,
			 -1.0, -1.0,  -1.0,
			     
			 -1.0,  1.0, -1.0,
			  1.0,  1.0, -1.0,
			 -1.0, -1.0, -1.0,
			  1.0, -1.0, -1.0,

			 -1.0,  1.0,  1.0,
			  1.0,  1.0,  1.0,
			 -1.0,  1.0, -1.0,
			  1.0,  1.0, -1.0,
			
};


static GLuint indices[] = {0, 1, 2,
			   1, 2, 3,

			   4, 5, 6,
			   5, 6, 7,

			   8, 9, 10,
			   9, 10, 11,

			   12, 13, 14,
			   13, 14, 15,

			   16, 17, 18,
			   17, 18, 19,

			   20, 21, 22,
			   21, 22, 23};

static const char vs_src[] =
"#version 330\n\
in vec3 position;\n\
in vec3 uvs;\n\
out vec4 color;\n\
out vec3 tex_coord;\n\
uniform mat4 world;\n\
void main() {\n\
    tex_coord = uvs;\n\
    gl_Position = world * vec4(position, 1.0);\n\
}";

static const char fs_src[] =
"#version 330\n\
in  vec3 tex_coord;\n\
out vec4 out_color;\n\
uniform samplerCube frameTexture;\n\
void main() {\n\
    out_color = texture(frameTexture, tex_coord);\n\
}";

CubeRenderer::CubeRenderer(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;
	this->bRenderCube = true;
	Init();
}

CubeRenderer::~CubeRenderer()
{
	delete pAppCamera;
	delete pCubeCamera;
	glDeleteProgram(m_program);
	glDeleteBuffers(2, m_vbos);
	glDeleteBuffers(1, &m_ibo);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteTextures(1, &m_color);
	glDeleteTextures(1, &m_depth);
	glDeleteFramebuffers(NUM_SIDES, m_fbos);
}

GLResult CubeRenderer::Init()
{
	GLResult result = GLResult::Success;
	GLuint vs, fs;

	result = CompileShader(vs_src, GL_VERTEX_SHADER, &vs);
	result = CompileShader(fs_src, GL_FRAGMENT_SHADER, &fs);
	result = LinkProgram(vs, fs, &m_program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	glBindFragDataLocation(m_program,
			       0,
			       "out_color");
	
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	
	glGenBuffers(2, m_vbos);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbos[0]);
	glBufferData(GL_ARRAY_BUFFER,
		     sizeof(vertices),
		     vertices,
		     GL_STATIC_DRAW);

	GLint pos_attr = glGetAttribLocation(m_program, "position");
	if (pos_attr == -1) {
		return GLResult::Error;
	}
	glEnableVertexAttribArray(pos_attr);
	glVertexAttribPointer(pos_attr,
			      3,
			      GL_FLOAT,
			      GL_FALSE,
			      0,
			      nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbos[1]);
	glBufferData(GL_ARRAY_BUFFER,
		     sizeof(uvs),
		     uvs,
		     GL_STATIC_DRAW);

	GLint uv_attr = glGetAttribLocation(m_program, "uvs");
	if (uv_attr == -1) {
		return GLResult::Error;
	}
	glEnableVertexAttribArray(uv_attr);
	glVertexAttribPointer(uv_attr,
			      3,
			      GL_FLOAT,
			      GL_FALSE,
			      0,
			      nullptr);

	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		     sizeof(indices),
		     indices,
		     GL_STATIC_DRAW);

	// allocate textures and fbos for attachment
	// make one per face
	glGenTextures(1, &m_color);
	glGenTextures(1, &m_depth);

	// +x
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_color);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		     0,
		     GL_RGBA,
		     512, 512,
		     0,
		     GL_RGBA,
		     GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_depth);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		     0,
		     GL_DEPTH_COMPONENT24,
		     512, 512,
		     0,
		     GL_DEPTH_COMPONENT,
		     GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	// -x
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_color);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		     0,
		     GL_RGBA,
		     512, 512,
		     0,
		     GL_RGBA,
		     GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_depth);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		     0,
		     GL_DEPTH_COMPONENT24,
		     512, 512,
		     0,
		     GL_DEPTH_COMPONENT,
		     GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	// +y
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_color);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		     0,
		     GL_RGBA,
		     512, 512,
		     0,
		     GL_RGBA,
		     GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_depth);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		     0,
		     GL_DEPTH_COMPONENT24,
		     512, 512,
		     0,
		     GL_DEPTH_COMPONENT,
		     GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	// -y
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_color);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		     0,
		     GL_RGBA,
		     512, 512,
		     0,
		     GL_RGBA,
		     GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_depth);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		     0,
		     GL_DEPTH_COMPONENT24,
		     512, 512,
		     0,
		     GL_DEPTH_COMPONENT,
		     GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	// +z
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_color);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		     0,
		     GL_RGBA,
		     512, 512,
		     0,
		     GL_RGBA,
		     GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_depth);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		     0,
		     GL_DEPTH_COMPONENT24,
		     512, 512,
		     0,
		     GL_DEPTH_COMPONENT,
		     GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	// -z
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_color);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		     0,
		     GL_RGBA,
		     512, 512,
		     0,
		     GL_RGBA,
		     GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_depth);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		     0,
		     GL_DEPTH_COMPONENT24,
		     512, 512,
		     0,
		     GL_DEPTH_COMPONENT,
		     GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	glGenFramebuffers(NUM_SIDES, m_fbos);
	GLenum attachments[2] = {GL_COLOR_ATTACHMENT0};

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[0]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_color, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_depth, 0);
	glDrawBuffers(2, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[1]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, m_color, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, m_depth, 0);
	glDrawBuffers(2, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[2]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, m_color, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, m_depth, 0);
	glDrawBuffers(2, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[3]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, m_color, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, m_depth, 0);
	glDrawBuffers(2, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[4]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, m_color, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, m_depth, 0);
	glDrawBuffers(2, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[5]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, m_color, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, m_depth, 0);
	glDrawBuffers(2, attachments);

	// allocate cameras
	// change to ortho, make one per face
	pCubeCamera = new Camera(512.0, 512.0,
				 3.14 / 4.0,
				 0.1, //these should be 
				 10000.0,
				 2.0);

	pCubeCamera->Move(glm::vec3(0.0, 1.5, 0.0));
	pCubeCamera->SetPerspective(false);

	pAppCamera = new Camera(static_cast<float>(m_width),
				static_cast<float>(m_height),
				3.14 / 4.0,
				0.1,
				10000.0,
				1.0);
	pAppCamera->Move(glm::vec3(2.5, 3.0, 0.0));
	return result;
}

void CubeRenderer::Resize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;
	pAppCamera->Resize(static_cast<float>(width), static_cast<float>(height));
}

void CubeRenderer::Step(uint32_t stepMs)
{
	pAppCamera->Step(stepMs);
}

static glm::vec3 up1 = glm::vec3(0.0f, 1.0f, 0.0f);
static glm::vec3 up2 = glm::vec3(0.0f, 0.0f, 1.0f);

void CubeRenderer::RenderCube(Scene *pTargetScene)
{

	for (uint8_t i=0; i < NUM_SIDES; ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[i]);
		glViewport(0,0,512,512);
		//move camera
		// todo, ugh, and I made more complicated with swapping X
		switch(i)
		{
		case 0:
			//should be scale and center
			pCubeCamera->SetPosition(glm::vec3(-3.0, 1.5f, -5.0f));
			pCubeCamera->Target(glm::vec3(0.0f, 1.5f, -5.0f), up1);
			break;
		case 1:
			pCubeCamera->SetPosition(glm::vec3(3.0, 1.5f, -5.0f));
			pCubeCamera->Target(glm::vec3(0.0f, 1.5f, -5.0f), up1);
			break;
		case 2:
			pCubeCamera->SetPosition(glm::vec3(0.0, 4.5f, -5.0f));
			pCubeCamera->Target(glm::vec3(0.0f, 1.5f, -5.0f), -1.0f * up2);
			break;
		case 3:
			pCubeCamera->SetPosition(glm::vec3(0.0, -2.5f, -5.0f));
			pCubeCamera->Target(glm::vec3(0.0f, 1.5f, -5.0f), up2);
			break;
		case 4:
			//should be scale and center
			pCubeCamera->SetPosition(glm::vec3(0.0, 1.5f, -2.0f));
			pCubeCamera->Target(glm::vec3(0.0f, 1.5f, -5.0f), up1);
			break;
		case 5:
			pCubeCamera->SetPosition(glm::vec3(0.0, 1.5f, -8.0f));
			pCubeCamera->Target(glm::vec3(0.0f, 1.5f, -5.0f), up1);
			break;

		}

		pTargetScene->Render(pCubeCamera);
	}


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// resize...
	glViewport(0,0,m_width,m_height);
	
	GLint world_uniform = glGetUniformLocation(m_program, "world");
	if (world_uniform == -1) {
		fprintf(stderr, "[ERROR] could not get uniform location");
	}

	glClearColor(0.2, 0.3, 0.2, 1.0);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	glm::mat4 model = glm::translate(glm::vec3(0.0, 1.5, -5.0));
	glm::mat4 view = pAppCamera->View();
	glm::mat4 model_view_projection = pAppCamera->Projection() * view * model;

	glClear(GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_program);
	glUniformMatrix4fv(world_uniform,
			   1,
			   GL_FALSE,
			   glm::value_ptr(model_view_projection));

	//attach texture(s)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_color);

	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES,
		       36,
		       GL_UNSIGNED_INT,
		       nullptr);
	glBindVertexArray(0);
}

void CubeRenderer::RenderScene(Scene *pTargetScene)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// resize...
	glViewport(0,0,m_width,m_height);
        pTargetScene->Render(pAppCamera);
}

void CubeRenderer::Render(Scene *pTargetScene)
{
	if (bRenderCube != false)
		RenderCube(pTargetScene);
	else
		RenderScene(pTargetScene);
}

bool CubeRenderer::HandleInputEvent(SDL_Event event)
{
	bool result = false;
	if (event.type == SDL_KEYUP)
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_v:
			this->bRenderCube = !this->bRenderCube;
			result = true;
			break;
		}
	}

	if (result == false)
		result = pAppCamera->HandleInputEvent(event);

	return result;
}
