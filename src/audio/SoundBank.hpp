#ifndef BLANK_AUDIO_SOUNDBANK_HPP_
#define BLANK_AUDIO_SOUNDBANK_HPP_

#include "Sound.hpp"

#include <vector>


namespace blank {

class AssetLoader;
class Audio;
class ResourceIndex;

class SoundBank {

public:
	SoundBank();

	void Load(const AssetLoader &, const ResourceIndex &);

	const Sound &operator [](std::size_t i) const noexcept { return sounds[i]; }

private:
	std::vector<Sound> sounds;

};

}

#endif
