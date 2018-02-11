#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char* filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		assert(false);
	}

	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return retTexture;
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

		glViewport(0, 0, 640, 360);
		ShaderProgram program;
		program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

		GLuint earth = LoadTexture(RESOURCE_FOLDER"Earth.png");
		GLuint mars = LoadTexture(RESOURCE_FOLDER"Mars.png");
		GLuint moon = LoadTexture(RESOURCE_FOLDER"Moon.png");

		Matrix projectionMatrix;
		Matrix modelMatrix;
		Matrix modelMatrix2;
		Matrix modelMatrix3;
		Matrix modelMatrix4;
		Matrix viewMatrix;

		projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

		glUseProgram(program.programID);

		float lastFrameTicks = 0.0f;

		glUseProgram(program.programID);

		SDL_Event event;
		bool done = false;
		while (!done) {
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
			}
			float ticks = (float)SDL_GetTicks() / 100.0f;
			float elapsed = ticks - lastFrameTicks;
			lastFrameTicks = ticks;

			glClear(GL_COLOR_BUFFER_BIT);

			program.SetModelMatrix(modelMatrix);
			program.SetProjectionMatrix(projectionMatrix);
			program.SetViewMatrix(viewMatrix);

			modelMatrix.Identity();
			modelMatrix.Scale(1.0f, 1.0f, 1.0f);
			modelMatrix.Translate(-2.0f, 1.0f, 0.0f);
			glBindTexture(GL_TEXTURE_2D, earth);

			float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(program.positionAttribute);

			float texCoords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
			glEnableVertexAttribArray(program.texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glBindTexture(GL_TEXTURE_2D, mars);
			program.SetModelMatrix(modelMatrix2);
			modelMatrix2.Rotate(elapsed*sinf(45) * 0.1);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glBindTexture(GL_TEXTURE_2D, moon);
			program.SetModelMatrix(modelMatrix3);
			modelMatrix3.Identity();
			modelMatrix3.Translate(2.0f, -1.0f, 0.0f);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			SDL_GL_SwapWindow(displayWindow);
		}

		SDL_Quit();
		return 0;
}
