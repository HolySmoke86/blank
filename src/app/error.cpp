#include "error.hpp"

#include <alut.h>
#include <cerrno>
#include <cstring>
#include <SDL.h>
#include <SDL_net.h>
#include <SDL_ttf.h>
#include <GL/glew.h>

using namespace std;


namespace {

string alut_error_append(ALenum num, string msg) {
	const char *error = alutGetErrorString(num);
	if (error && *error != '\0') {
		msg += ": ";
		msg += error;
	}
	return msg;
}

string gl_error_append(string msg) {
	const GLubyte *error = gluErrorString(glGetError());
	if (error && *error != '\0') {
		const GLubyte *errEnd = error;
		while (*errEnd != '\0') {
			++errEnd;
		}
		msg += ": ";
		msg.append(error, errEnd);
	}
	return msg;
}

string gl_error_get() {
	string msg;
	const GLubyte *error = gluErrorString(glGetError());
	if (error && *error != '\0') {
		const GLubyte *errEnd = error;
		while (*errEnd != '\0') {
			++errEnd;
		}
		msg.assign(error, errEnd);
	}
	return msg;
}

string net_error_append(string msg) {
	const char *error = SDLNet_GetError();
	if (*error != '\0') {
		msg += ": ";
		msg += error;
	}
	return msg;
}

string sdl_error_append(string msg) {
	const char *error = SDL_GetError();
	if (error && *error != '\0') {
		msg += ": ";
		msg += error;
		SDL_ClearError();
	}
	return msg;
}

string ttf_error_append(string msg) {
	const char *error = TTF_GetError();
	if (error && *error != '\0') {
		msg += ": ";
		msg += error;
	}
	return msg;
}

}


namespace blank {

AlutError::AlutError(ALenum num)
: runtime_error(alutGetErrorString(num)) {

}

AlutError::AlutError(ALenum num, const string &msg)
: runtime_error(alut_error_append(num, msg)) {

}


GLError::GLError()
: runtime_error(gl_error_get()) {

}

GLError::GLError(const string &msg)
: runtime_error(gl_error_append(msg)) {

}


NetError::NetError()
: runtime_error(SDLNet_GetError()) {

}

NetError::NetError(const string &msg)
: runtime_error(net_error_append(msg)) {

}


SDLError::SDLError()
: runtime_error(SDL_GetError()) {

}

SDLError::SDLError(const string &msg)
: runtime_error(sdl_error_append(msg)) {

}


SysError::SysError()
: SysError(errno) {

}

SysError::SysError(const string &msg)
: SysError(errno, msg) {

}

SysError::SysError(int err_num)
: runtime_error(strerror(err_num)) {

}

SysError::SysError(int err_num, const string &msg)
: runtime_error(msg + ": " + strerror(err_num)) {

}


TTFError::TTFError()
: runtime_error(TTF_GetError()) {

}

TTFError::TTFError(const string &msg)
: runtime_error(ttf_error_append(msg)) {

}

}
