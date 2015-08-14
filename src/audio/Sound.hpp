#ifndef BLANK_AUDIO_SOUND_HPP_
#define BLANK_AUDIO_SOUND_HPP_

#include <al.h>


namespace blank {

class Sound {

public:
	Sound();
	explicit Sound(const char *);
	~Sound();

	Sound(Sound &&);
	Sound &operator =(Sound &&);

	Sound(const Sound &) = delete;
	Sound &operator =(const Sound &) = delete;

public:
	void Bind(ALuint src) const;

	/// full duration in milliseconds
	int Duration() const noexcept { return duration; }

private:
	ALuint handle;
	int duration;

};

}

#endif
