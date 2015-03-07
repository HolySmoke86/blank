#include "init.hpp"

#include <algorithm>
#include <SDL.h>
#include <SDL_image.h>
#include <stdexcept>
#include <string>
#include <GL/glew.h>


namespace {

void sdl_error(std::string msg) {
	const char *error = SDL_GetError();
	if (*error != '\0') {
		msg += ": ";
		msg += error;
		SDL_ClearError();
	}
	throw std::runtime_error(msg);
}

}

namespace blank {

InitSDL::InitSDL() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		sdl_error("SDL_Init(SDL_INIT_VIDEO)");
	}
}

InitSDL::~InitSDL() {
	SDL_Quit();
}


InitIMG::InitIMG() {
	if (IMG_Init(IMG_INIT_PNG) == 0) {
		sdl_error("IMG_Init(IMG_INIT_PNG)");
	}
}

InitIMG::~InitIMG() {
	IMG_Quit();
}


InitGL::InitGL() {
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) != 0) {
		sdl_error("SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)");
	}
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) != 0) {
		sdl_error("SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3)");
	}
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) != 0) {
		sdl_error("SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)");
	}

	if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) != 0) {
		sdl_error("SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)");
	}
}


Window::Window()
: handle(SDL_CreateWindow(
	"blank",
	SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	960, 600,
	SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
)) {
	if (!handle) {
		sdl_error("SDL_CreateWindow");
	}
}

Window::~Window() {
	SDL_DestroyWindow(handle);
}

void Window::GrabInput() {
	SDL_SetWindowGrab(handle, SDL_TRUE);
}

void Window::ReleaseInput() {
	SDL_SetWindowGrab(handle, SDL_FALSE);
}

void Window::GrabMouse() {
	if (SDL_SetRelativeMouseMode(SDL_TRUE) != 0) {
		sdl_error("SDL_SetRelativeMouseMode");
	}
}

void Window::ReleaseMouse() {
	if (SDL_SetRelativeMouseMode(SDL_FALSE) != 0) {
		sdl_error("SDL_SetRelativeMouseMode");
	}
}

GLContext Window::CreateContext() {
	return GLContext(handle);
}

void Window::Flip() {
	SDL_GL_SwapWindow(handle);
}


GLContext::GLContext(SDL_Window *win)
: handle(SDL_GL_CreateContext(win)) {
	if (!handle) {
		sdl_error("SDL_GL_CreateContext");
	}
}

GLContext::~GLContext() {
	if (handle) {
		SDL_GL_DeleteContext(handle);
	}
}


GLContext::GLContext(GLContext &&other)
: handle(other.handle) {
	other.handle = nullptr;
}

GLContext &GLContext::operator =(GLContext &&other) {
	std::swap(handle, other.handle);
	return *this;
}

void GLContext::EnableVSync() {
	if (SDL_GL_SetSwapInterval(1) != 0) {
		sdl_error("SDL_GL_SetSwapInterval");
	}
}

void GLContext::EnableDepthTest() {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void GLContext::EnableBackfaceCulling() {
	glEnable(GL_CULL_FACE);
}

void GLContext::Clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLContext::ClearDepthBuffer() {
	glClear(GL_DEPTH_BUFFER_BIT);
}


InitGLEW::InitGLEW() {
	glewExperimental = GL_TRUE;
	GLenum glew_err = glewInit();
	if (glew_err != GLEW_OK) {
		std::string msg("glewInit: ");
		const GLubyte *errBegin = glewGetErrorString(glew_err);
		const GLubyte *errEnd = errBegin;
		while (*errEnd != '\0') {
			++errEnd;
		}
		msg.append(errBegin, errEnd);
		throw std::runtime_error(msg);
	}
}

}
