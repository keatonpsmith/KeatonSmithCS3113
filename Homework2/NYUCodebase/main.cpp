#ifdef _WINDOWS
	#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "ShaderProgram.h"
#include "Matrix.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

class Racket {
private:
	int score = 0;
	float left, right, top, bottom;
public:
	Racket(float l, float r, float t, float b) : left(l), right(r), top(t), bottom(b) {}
	float getLeft() { return left; }
	void setLeft(float l) { left = l; }
	float getRight() { return right; }
	void setRight(float r) { right = r; }
	float getTop() { return top; }
	void setTop(float t) { top = t; }
	float getBottom() { return bottom; }
	void setBottom(float b) { bottom = b; }
	int getScore() { return score; }
	void addScore() { score++; }
};

class Ball {
private:
	float xPos = 0.0f, yPos = 0.0f, vel = 0.4f, accel = 5.0f;
	float xDir = 3.0f, yDir = 3.0f;
public:
	Ball() {}
	Ball(float xP, float yP, float v, float a, float xD, float yD) : xPos(xP), yPos(yP), vel(v), accel(a), xDir(xD), yDir(yD) {}
	void refresh() {
		xPos = 0.0f;
		yPos = 0.0f;
		vel = 0.5f;
		accel = 10.0f;
		xDir = 3.0f;
		xDir = 3.0f;
	}
	float getXDir() { return xDir; }
	void setXDir(float d) { xDir = d; }
	float getYDir() { return yDir; }
	void setYDir(float d) { yDir = d; }
	float getXPos() { return xPos; }
	void setXPos(float p) { xPos = p; }
	float getYPos() { return yPos; }
	void setYPos(float p) { yPos = p; }
	float getVel() { return vel; }
	void setVel(float v) { vel = v; }
	float getAccel() { return accel; }
	void setAccel(float a) { accel = a; }
	void move(float elap) {
		xPos += (vel * xDir * elap);
		yPos += (vel * yDir * elap);
	}
};

GLuint LoadTexture(const char *filepath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filepath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		//assert(false);
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

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif
	ShaderProgram program;
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint blank = LoadTexture(RESOURCE_FOLDER"Blank.png");

	Matrix projectionMatrix;
	Matrix leftRacket;
	Matrix rightRacket;
	Matrix ballMatrix;
	Matrix viewMatrix;
	projectionMatrix.SetOrthoProjection(-4.0f, 4.0f, -2.25f, 2.25f, -1.0f, 1.0f);

	float lastFrameTicks = 0.0f;

	Racket left(-3.7f, -3.6f, 0.5f, -0.5f);
	Racket right(3.6f, 3.7f, 0.5f, -0.5f);
	Ball ball = Ball();

	SDL_Event event;
	bool done = false, winner = false;
	while (!done) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_W && left.getTop() < 2.25f) {
					left.setTop(left.getTop() + 0.1f);
					left.setBottom(left.getBottom() + 0.1f);
					leftRacket.Translate(0.0f, 0.1f, 0.0f);
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_S && left.getBottom() > -2.25f) {
					left.setTop(left.getTop() - 0.1f);
					left.setBottom(left.getBottom() - 0.1f);
					leftRacket.Translate(0.0f, -0.1f, 0.0f);
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_UP && right.getTop() < 2.25f) {
					right.setTop(right.getTop() + 0.1f);
					right.setBottom(right.getBottom() + 0.1f);
					rightRacket.Translate(0.0f, 0.1f, 0.0f);
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN && right.getBottom() > -2.25f) {
					right.setTop(right.getTop() - 0.1f);
					right.setBottom(right.getBottom() - 0.1f);
					rightRacket.Translate(0.0f, -0.1f, 0.0f);
				}
			}
		}

		program.SetModelMatrix(leftRacket);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetViewMatrix(viewMatrix);

		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(program.programID);

		float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

		float leftRacketVertices[] = { -3.7f, -0.5f, -3.6f, -0.5f, -3.6f, 0.5f, -3.6f, 0.5f, -3.7f, 0.5f, -3.7f, -0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, leftRacketVertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glBindTexture(GL_TEXTURE_2D, blank);
		
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		program.SetModelMatrix(rightRacket);
		float rightRacketVertices[] = { 3.6f, -0.5f, 3.7f, -0.5f, 3.6f, 0.5f, 3.7f, 0.5f, 3.6f, 0.5f, 3.7f, -0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, rightRacketVertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glBindTexture(GL_TEXTURE_2D, blank);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		program.SetModelMatrix(ballMatrix);
		float ballVertices[] = { -0.1f, -0.1f, 0.1f, -0.1f, 0.1f, 0.1f, 0.1f, 0.1f, -0.1f, 0.1f, -0.1f, -0.1f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballVertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glBindTexture(GL_TEXTURE_2D, blank);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		if (!winner) {
			if (ball.getXPos() <= left.getRight() && ball.getYPos() <= left.getTop() && ball.getYPos() >= left.getBottom() || ball.getXPos() >= right.getLeft() && ball.getYPos() <= right.getTop() && ball.getYPos() >= right.getBottom()) {
				ball.setXDir(ball.getXDir() * -1);
				ball.setVel(ball.getVel() + (ball.getAccel() * elapsed));
				ball.move(elapsed);
				ballMatrix.Translate((ball.getVel() * ball.getXDir() * elapsed), (ball.getVel() * ball.getYDir() * elapsed), 0.0f);
			}
			else if (ball.getYPos() + 0.1f >= 2.25f || ball.getYPos() - 0.1f <= -2.25f) {
				ball.setYDir(ball.getYDir() * -1);
				ball.setVel(ball.getVel() + ball.getAccel() * elapsed);
				ball.move(elapsed);
				ballMatrix.Translate((ball.getVel() * ball.getXDir() * elapsed), (ball.getVel() * ball.getYDir() * elapsed), 0.0f);
			}
			else if (ball.getXPos() >= right.getRight()) {
				left.addScore();
				ballMatrix.Translate(-ball.getXPos(), -ball.getYPos(), 0.0f);
				ball.refresh();
			}
			else if (ball.getXPos() <= left.getLeft()) {
				right.addScore();
				ballMatrix.Translate(-ball.getXPos(), -ball.getYPos(), 0.0f);
				ball.refresh();
			}
			else {
				ball.move(elapsed);
				ballMatrix.Translate((ball.getVel() * ball.getXDir() * elapsed), (ball.getVel() * ball.getYDir() * elapsed), 0.0f);
			}
			if (left.getScore() > 2 || right.getScore() > 2) {
				winner = true;
			}
		}
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}