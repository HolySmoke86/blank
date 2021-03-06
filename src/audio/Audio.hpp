#ifndef BLANK_AUDIO_AUDIO_HPP_
#define BLANK_AUDIO_AUDIO_HPP_

#include "../app/IntervalTimer.hpp"
#include "../graphics/glm.hpp"

#include <al.h>


namespace blank {

class Sound;

class Audio {

public:
	Audio();
	~Audio();

	Audio(const Audio &) = delete;
	Audio &operator =(const Audio &) = delete;

public:
	void Position(const glm::vec3 &) noexcept;
	void Velocity(const glm::vec3 &) noexcept;
	void Orientation(const glm::vec3 &dir, const glm::vec3 &up) noexcept;

	void Play(
		const Sound &,
		const glm::vec3 &pos = glm::vec3(0.0f),
		const glm::vec3 &vel = glm::vec3(0.0f),
		const glm::vec3 &dir = glm::vec3(0.0f)
	) noexcept;

	void StopAll() noexcept;

	void Update(int dt) noexcept;

private:
	int NextFree() noexcept;

private:
	static constexpr std::size_t NUM_SRC = 16;
	ALuint source[NUM_SRC];
	CoarseTimer timer[NUM_SRC];
	int last_free;

};

}

#endif
