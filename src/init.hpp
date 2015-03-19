#ifndef BLANK_INIT_HPP_
#define BLANK_INIT_HPP_

#include <SDL.h>


namespace blank {

class GLContext;


class InitSDL {

public:
	InitSDL();
	~InitSDL();

	InitSDL(const InitSDL &) = delete;
	InitSDL &operator =(const InitSDL &) = delete;

};


class InitIMG {

public:
	InitIMG();
	~InitIMG();

	InitIMG(const InitIMG &) = delete;
	InitIMG &operator =(const InitIMG &) = delete;

};


class InitGL {

public:
	explicit InitGL(bool double_buffer = true, int sample_size = 1);

	InitGL(const InitGL &) = delete;
	InitGL &operator =(const InitGL &) = delete;

};


class Window {

public:
	Window();
	~Window();

	Window(const Window &) = delete;
	Window &operator =(const Window &) = delete;

	void GrabInput();
	void ReleaseInput();

	void GrabMouse();
	void ReleaseMouse();

	GLContext CreateContext();

	void Flip();

private:
	SDL_Window *handle;

};


class GLContext {

public:
	explicit GLContext(SDL_Window *);
	~GLContext();

	GLContext(GLContext &&);
	GLContext &operator =(GLContext &&);

	GLContext(const GLContext &) = delete;
	GLContext &operator =(const GLContext &) = delete;

	static void EnableVSync();
	static void EnableDepthTest();
	static void EnableBackfaceCulling();

	static void Clear();
	static void ClearDepthBuffer();

private:
	SDL_GLContext handle;

};


class InitGLEW {

public:
	InitGLEW();

	InitGLEW(const InitGLEW &) = delete;
	InitGLEW &operator =(const InitGLEW &) = delete;

};

}

#endif
