#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <vector>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

using namespace std;

SDL_Window* displayWindow;

Matrix modelMatrix;
Matrix projectionMatrix;
Matrix viewMatrix;

ShaderProgram program;
GLuint font;
GLuint spriteSheet;

enum GameState {TITLE_SCREEN, GAME_LEVEL};
int state = TITLE_SCREEN;
bool playing = true;
float elapsed;

GLuint loadTexture(const char* filePath);

void draw(ShaderProgram program, float vertices[], float coords[]);

void drawText(ShaderProgram program, int fontTexture, std::string text, float size, float spacing);

void renderTitle();

void renderGame();

void update(float elapsed);

class Entity {
public:
	float posX, posY, dx, dy;
	float top, bottom, left, right;
	float u, v, width, height;
	Matrix myMatrix;
	Entity() {};
	Entity(float x, float y, float dx, float dy, float u, float v, float w, float h) : posX(x), posY(y), dx(dx), dy(dy), u(u), v(v), width(w), height(h) {
		top = y + .05;
		bottom = y - .05;
		right = x + .05;
		left = x - .05;
		myMatrix.Identity();
		myMatrix.Translate(x, y, 0.0f);
	}
	void draw() {
		myMatrix.Identity();
		myMatrix.Translate(posX, posY, 0.0f);
		program.SetModelMatrix(myMatrix);
		float vertices[] = { -0.15, -0.15, 0.15, -0.15, 0.15, 0.15, -0.15, -0.15, 0.15, 0.15, -0.15, 0.15 };
		float texCoords[] = { u, v + height, u + width, v, u, v, u + width, v, u, v + height, u + width, v + height };
		glUseProgram(program.programID);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glBindTexture(GL_TEXTURE_2D, spriteSheet);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
	}

};

Entity player;
vector<Entity> invaders;
vector<Entity> goodLasers;
vector<Entity> badLasers;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
	glewInit();
	#endif

	glViewport(0, 0, 640, 360);
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	SDL_Event event;
	bool done = false;

	projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);

	float lastFrameTicks = 0.0f;
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;

	player = Entity(0.0f, -1.5f, .2f, 0.0f, (float)((82 % 15) / 15.0), (float)((82) / 15.0) / 10.0, 1.0 / 30.0, 1.0 / 15.0);
	for (int i = 0; i<10; ++i) {
		invaders.push_back(Entity(-3.4 + (i % 5), 1.5 - (i / 5), 0.002f, .0001f, (float)((0 % 15) / 15.0), (float)((0) / 15.0) / 10.0, 1.0 / 15.0, 1.0 / 10.0));
	}

	glUseProgram(program.programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(.0f, .0f, .0f, 1.0f);
	font = loadTexture("font1.png");
	spriteSheet = loadTexture("sheet.png");

	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					if (state == TITLE_SCREEN) {
						state = GAME_LEVEL;
					}
					else {
						goodLasers.push_back(Entity(player.posX, player.posY, 0.0f, 0.2, (float)((12 % 15) / 15.0), (float)((12) / 15.0) / 10.0, 1.0 / 15.0, 1.0 / 10.0));
					}
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
					player.posX += .1f * elapsed;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
					player.posX -= .1f * elapsed;
				}
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		program.SetModelMatrix(modelMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetViewMatrix(viewMatrix);

		if (playing) {
			if (state == TITLE_SCREEN) {
				renderTitle();
			}
			else if (state == GAME_LEVEL) {
				update(elapsed);
				renderGame();
			}
		}
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}

GLuint loadTexture(const char* filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image\n";
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

void draw(ShaderProgram program, float vertices[], float coords[]) {
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, coords);
	glEnableVertexAttribArray(program.texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

void drawText(ShaderProgram program, int fontTexture, string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	vector<float> vertexData;
	vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
		int spriteIndex = (int)text[i];
		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {((size + spacing) * i) + (-0.5f * size), 0.5f * size, ((size + spacing) * i) + (-0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), 0.5f * size, ((size + spacing) * i) + (0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), 0.5f * size, ((size + spacing) * i) + (-0.5f * size), -0.5f * size,});
		texCoordData.insert(texCoordData.end(), {texture_x, texture_y, texture_x, texture_y + texture_size, texture_x + texture_size, texture_y, texture_x + texture_size, texture_y + texture_size, texture_x + texture_size, texture_y, texture_x, texture_y + texture_size});
	}
	glUseProgram(program.programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, (int)text.size() * 6);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

void renderTitle() {
	modelMatrix.Identity();
	modelMatrix.Translate(-2.5f, 0.5f, 0.0f);
	program.SetModelMatrix(modelMatrix);
	drawText(program, font, "Galaxy Attackers", 0.3f, 0.0f);
	modelMatrix.Translate(-0.25f, -1.0f, 0.0f);
	program.SetModelMatrix(modelMatrix);
	drawText(program, font, "Press SPACE when you are ready.", 0.2f, 0.0f);
}

void renderGame() {
	player.draw();
	for (size_t i = 0; i < invaders.size(); ++i) {
		invaders[i].draw();
	}
	for (size_t i = 0; i < goodLasers.size(); ++i) {
		goodLasers[i].draw();
	}
	for (size_t i = 0; i < badLasers.size(); ++i) {
		badLasers[i].draw();
	}
}

void update(float elapsed) {
	for (size_t i = 0; i < invaders.size(); ++i) {
		invaders[i].posX += invaders[i].dx * elapsed;
		invaders[i].posY -= invaders[i].dy * elapsed;
		invaders[i].top -= invaders[i].dy * elapsed;
		invaders[i].bottom -= invaders[i].dy * elapsed;
		invaders[i].left += invaders[i].dx * elapsed;
		invaders[i].right += invaders[i].dx * elapsed;
		if ((invaders[i].posX > 3.5 && invaders[i].dx != 0) || (invaders[i].posX < -3.5 && invaders[i].dx != 0)) {
			invaders[i].dx *= -1;
		}
		if (invaders[i].bottom <= player.top && invaders[i].top >= player.bottom && invaders[i].left <= player.right && invaders[i].right >= player.left) {
			playing = false;
		}
	}
	for (size_t i = 0; i < goodLasers.size(); ++i) {
		goodLasers[i].posY += goodLasers[i].dy * elapsed;
		goodLasers[i].top += goodLasers[i].dy * elapsed;
		goodLasers[i].bottom += goodLasers[i].dy * elapsed;
		for (size_t j = 0; j < invaders.size(); ++j) {
			if (goodLasers[i].bottom <= invaders[j].top && goodLasers[i].top <= invaders[j].bottom && goodLasers[i].left <= invaders[j].right && goodLasers[i].right <= invaders[j].left) {
				invaders.erase(invaders.begin() + j);
			}
		}
	}
}
