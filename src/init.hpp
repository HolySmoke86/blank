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


class InitGL {

public:
	InitGL();
	~InitGL();

	InitGL(const InitGL &) = delete;
	InitGL &operator =(const InitGL &) = delete;

};


class Window {

public:
	Window();
	~Window();

	Window(const Window &) = delete;
	Window &operator =(const Window &) = delete;

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

private:
	SDL_GLContext handle;

};


class InitGLEW {

public:
	InitGLEW();
	~InitGLEW();

	InitGLEW(const InitGLEW &) = delete;
	InitGLEW &operator =(const InitGLEW &) = delete;

};

}

#endif
