#include "ALError.hpp"
#include "Audio.hpp"
#include "Sound.hpp"

#include <algorithm>
#include <alut.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>


namespace {

const char *al_error_string(ALenum num) {
	switch (num) {
		case AL_NO_ERROR:
			return "no error";
		case AL_INVALID_NAME:
			return "invalid name";
		case AL_INVALID_ENUM:
			return "invalid enum";
		case AL_INVALID_VALUE:
			return "invalid value";
		case AL_INVALID_OPERATION:
			return "invalid operation";
		case AL_OUT_OF_MEMORY:
			return "out of memory";
	}
	return "unknown AL error";
}

std::string al_error_append(ALenum num, std::string msg) {
	return msg + ": " + al_error_string(num);
}

}

namespace blank {

ALError::ALError(ALenum num)
: std::runtime_error(al_error_string(num)) {

}

ALError::ALError(ALenum num, const std::string &msg)
: std::runtime_error(al_error_append(num, msg)) {

}


Audio::Audio() {
	alGenSources(NUM_SRC, source);
	ALenum err = alGetError();
	if (err != AL_NO_ERROR) {
		throw ALError(err, "alGenSources");
	}
	for (std::size_t i = 0; i < NUM_SRC; ++i) {
		alSourcef(source[i], AL_REFERENCE_DISTANCE, 2.0f);
		alSourcef(source[i], AL_ROLLOFF_FACTOR, 1.0f);
	}
}

Audio::~Audio() {
	alDeleteSources(NUM_SRC, source);
	ALenum err = alGetError();
	if (err != AL_NO_ERROR) {
		std::cerr << "warning: alDeleteSources failed with " << al_error_string(err) << std::endl;
		//throw ALError(err, "alDeleteSources");
	}
}

void Audio::Position(const glm::vec3 &pos) noexcept {
	alListenerfv(AL_POSITION, glm::value_ptr(pos));
	//std::cout << "listener at " << pos << std::endl;
}

void Audio::Velocity(const glm::vec3 &vel) noexcept {
	alListenerfv(AL_VELOCITY, glm::value_ptr(vel));
}

void Audio::Orientation(const glm::vec3 &dir, const glm::vec3 &up) noexcept {
	ALfloat orient[6] = {
		dir.x, dir.y, dir.z,
		up.x, up.y, up.z,
	};
	alListenerfv(AL_ORIENTATION, orient);
}

void Audio::Play(
	const Sound &sound,
	const glm::vec3 &pos,
	const glm::vec3 &vel,
	const glm::vec3 &dir
) noexcept {
	// TODO: find next free source
	ALuint src = source[0];

	sound.Bind(src);
	alSourcefv(src, AL_POSITION, glm::value_ptr(pos));
	alSourcefv(src, AL_VELOCITY, glm::value_ptr(vel));
	alSourcefv(src, AL_DIRECTION, glm::value_ptr(dir));
	alSourcePlay(src);
}

void Audio::StopAll() noexcept {
	alSourceStopv(NUM_SRC, source);
	for (std::size_t i = 0; i < NUM_SRC; ++i) {
		alSourcei(source[i], AL_BUFFER, AL_NONE);
	}
}


Sound::Sound()
: handle(AL_NONE) {
	alGenBuffers(1, &handle);
	ALenum err = alGetError();
	if (err != AL_NO_ERROR) {
		throw ALError(err, "alGenBuffers");
	}
}

Sound::Sound(const char *file)
: handle(alutCreateBufferFromFile(file)) {
	if (handle == AL_NONE) {
		throw ALError(alGetError(), "alutCreateBufferFromFile");
	}
}

Sound::~Sound() {
	if (handle != AL_NONE) {
		alDeleteBuffers(1, &handle);
		ALenum err = alGetError();
		if (err != AL_NO_ERROR) {
			std::cerr << "warning: alDeleteBuffers failed with " << al_error_string(err) << std::endl;
			//throw ALError(err, "alDeleteBuffers");
		}
	}
}

Sound::Sound(Sound &&other)
: handle(other.handle) {
	other.handle = AL_NONE;
}

Sound &Sound::operator =(Sound &&other) {
	std::swap(handle, other.handle);
	return *this;
}

void Sound::Bind(ALuint src) const {
	alSourcei(src, AL_BUFFER, handle);
}

}
