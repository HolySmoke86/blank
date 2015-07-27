#ifndef BLANK_APP_INIT_HPP_
#define BLANK_APP_INIT_HPP_

#include <SDL.h>
#include <stdexcept>
#include <string>


namespace blank {

class SDLError
: public std::runtime_error {

public:
	SDLError();
	explicit SDLError(const std::string &);

};


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


class InitTTF {

public:
	InitTTF();
	~InitTTF();

	InitTTF(const InitTTF &) = delete;
	InitTTF &operator =(const InitTTF &) = delete;

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

	SDL_Window *Handle() { return handle; }

	void Flip();

private:
	SDL_Window *handle;

};


class GLContext {

public:
	explicit GLContext(SDL_Window *);
	~GLContext();

	GLContext(const GLContext &) = delete;
	GLContext &operator =(const GLContext &) = delete;

private:
	SDL_GLContext ctx;

};


class InitGLEW {

public:
	InitGLEW();

	InitGLEW(const InitGLEW &) = delete;
	InitGLEW &operator =(const InitGLEW &) = delete;

};


struct Init {

	Init(bool double_buffer = true, int sample_size = 1);

	InitSDL init_sdl;
	InitIMG init_img;
	InitTTF init_ttf;
	InitGL init_gl;
	Window window;
	GLContext ctx;
	InitGLEW init_glew;

};

}

#endif
