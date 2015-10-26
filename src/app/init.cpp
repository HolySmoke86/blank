#include "init.hpp"

#include <algorithm>
#include <alut.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include <SDL_ttf.h>
#include <GL/glew.h>


namespace {

std::string sdl_error_append(std::string msg) {
	const char *error = SDL_GetError();
	if (*error != '\0') {
		msg += ": ";
		msg += error;
		SDL_ClearError();
	}
	return msg;
}

std::string net_error_append(std::string msg) {
	const char *error = SDLNet_GetError();
	if (*error != '\0') {
		msg += ": ";
		msg += error;
	}
	return msg;
}

std::string alut_error_append(ALenum num, std::string msg) {
	const char *error = alutGetErrorString(num);
	if (*error != '\0') {
		msg += ": ";
		msg += error;
	}
	return msg;
}

}

namespace blank {

AlutError::AlutError(ALenum num)
: std::runtime_error(alutGetErrorString(num)) {

}

AlutError::AlutError(ALenum num, const std::string &msg)
: std::runtime_error(alut_error_append(num, msg)) {

}


NetError::NetError()
: std::runtime_error(SDLNet_GetError()) {

}

NetError::NetError(const std::string &msg)
: std::runtime_error(net_error_append(msg)) {

}


SDLError::SDLError()
: std::runtime_error(SDL_GetError()) {

}

SDLError::SDLError(const std::string &msg)
: std::runtime_error(sdl_error_append(msg)) {

}


InitSDL::InitSDL() {
	if (SDL_Init(SDL_INIT_EVENTS) != 0) {
		throw SDLError("SDL_Init(SDL_INIT_EVENTS)");
	}
}

InitSDL::~InitSDL() {
	SDL_Quit();
}


InitVideo::InitVideo() {
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		throw SDLError("SDL_InitSubSystem(SDL_INIT_VIDEO)");
	}
	// SDL seems to start out in text input state
	SDL_StopTextInput();
}

InitVideo::~InitVideo() {
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


InitIMG::InitIMG() {
	if (IMG_Init(IMG_INIT_PNG) == 0) {
		throw SDLError("IMG_Init(IMG_INIT_PNG)");
	}
}

InitIMG::~InitIMG() {
	IMG_Quit();
}


InitNet::InitNet() {
	if (SDLNet_Init() != 0) {
		throw SDLError("SDLNet_Init()");
	}
}

InitNet::~InitNet() {
	SDLNet_Quit();
}


InitTTF::InitTTF() {
	if (TTF_Init() != 0) {
		throw SDLError("TTF_Init()");
	}
}

InitTTF::~InitTTF() {
	TTF_Quit();
}


InitAL::InitAL() {
	if (!alutInit(nullptr, nullptr)) {
		throw AlutError(alutGetError(), "alutInit");
	}
}

InitAL::~InitAL() {
	if (!alutExit()) {
		throw AlutError(alutGetError(), "alutExit");
	}
}


InitGL::InitGL(bool double_buffer, int sample_size) {
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) != 0) {
		throw SDLError("SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)");
	}
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) != 0) {
		throw SDLError("SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3)");
	}
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) != 0) {
		throw SDLError("SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)");
	}

	if (double_buffer) {
		if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) != 0) {
			throw SDLError("SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)");
		}
	}

	if (sample_size > 1) {
		if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1) != 0) {
			throw SDLError("SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS)");
		}
		if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, sample_size) != 0) {
			throw SDLError("SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES)");
		}
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
		throw SDLError("SDL_CreateWindow");
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
		throw SDLError("SDL_SetRelativeMouseMode");
	}
}

void Window::ReleaseMouse() {
	if (SDL_SetRelativeMouseMode(SDL_FALSE) != 0) {
		throw SDLError("SDL_SetRelativeMouseMode");
	}
}

void Window::Flip() {
	SDL_GL_SwapWindow(handle);
}


GLContext::GLContext(SDL_Window *win)
: ctx(SDL_GL_CreateContext(win)) {
	if (!ctx) {
		throw SDLError("SDL_GL_CreateContext");
	}
}

GLContext::~GLContext() {
	SDL_GL_DeleteContext(ctx);
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


InitHeadless::InitHeadless()
: init_sdl()
, init_net() {

}

Init::Init(bool double_buffer, int sample_size)
: init_video()
, init_img()
, init_ttf()
, init_gl(double_buffer, sample_size)
, window()
, ctx(window.Handle())
, init_glew() {

}

}
