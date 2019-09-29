#include <GL/glew.h>
#include <math.h>

#include <SDL2/SDL.h>

#include "Camera.h"
#include "TestScene.h"
#include "GltfScene.h"
#include "CubeRenderer.h"
#include "log.h"

const char programName[] = "Cube Render";

SDL_Window *mainWindow;
SDL_GLContext mainContext;

bool SetOpenGLAttributes();
void PrintSDL_GL_Attributes();
void CheckSDLError();
void RunGame();
void Cleanup();

TestScene *testscene;
GltfScene *gltfscene;
CubeRenderer *cube;

bool Init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "Failed to init SDL\n");
		return false;
	}

	SetOpenGLAttributes();

	mainWindow = SDL_CreateWindow(programName,
				      SDL_WINDOWPOS_CENTERED,
				      SDL_WINDOWPOS_CENTERED,
				      1024, 768,
				      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!mainWindow)
	{
		fprintf(stderr, "Unable to create window\n");
		CheckSDLError();
		return false;
	}

	// Create our opengl context and attach it to our window
	mainContext = SDL_GL_CreateContext(mainWindow);

	if (!mainContext)
	{
		fprintf(stderr, "Unable to create OpenGL context\n");
		CheckSDLError();
		return false;
	}
	
	glewInit();

	// This makes our buffer swap syncronized with the monitor's vertical refresh
	SDL_GL_SetSwapInterval(1);
	SDL_SetRelativeMouseMode(SDL_TRUE);

#ifdef CUBE_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
#endif // CUBE_DEBUG

	testscene = new TestScene();
	gltfscene = new GltfScene("fox.gltf");
	cube = new CubeRenderer(1024, 768);

	return true;
}



bool SetOpenGLAttributes()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	return true;
}

int main(int argc, char *argv[])
{
	if (!Init())
		return -1;

	RunGame();

	Cleanup();

	return 0;
}

void RunGame()
{
	bool loop = true;

	while (loop)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type) {
			case SDL_QUIT:
				loop = false;
				break;
			case SDL_WINDOWEVENT:
				switch(event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					uint32_t width = event.window.data1;
					uint32_t height = event.window.data2;
					cube->Resize(width, height);
						   
					break;
				}
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					loop = false;
					break;
				default:
					cube->HandleInputEvent(event);
				}
				break;
			default:
				cube->HandleInputEvent(event);
			}
		}

		testscene->Step(16);
		cube->Step(16);

		cube->Render(gltfscene);

		SDL_GL_SwapWindow(mainWindow);
	}
}

void Cleanup()
{
	delete cube;
	delete testscene;
	delete gltfscene;

	SDL_GL_DeleteContext(mainContext);
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();
}

void CheckSDLError()
{
	const char *error = SDL_GetError();

	if (error[0] != '\0')
	{
		fprintf(stderr, "[ERROR] SLD Error : %s", error);
		SDL_ClearError();
	}
}

void PrintSDL_GL_Attributes()
{
	int value = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
	fprintf(stderr, "SDL_GL_CONTEXT_MAJOR_VERSION: %d", value);

	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
	fprintf(stderr, "SDL_GL_CONTEXT_MINOR_VERSION: %d", value);
}
