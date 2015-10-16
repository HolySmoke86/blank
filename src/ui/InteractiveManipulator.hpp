#ifndef BLANK_UI_INTERACTIVEMANIPULATOR_HPP_
#define BLANK_UI_INTERACTIVEMANIPULATOR_HPP_

#include "../world/WorldManipulator.hpp"

#include "../audio/Sound.hpp"


namespace blank {

class Audio;
class Entity;
class SoundBank;

class InteractiveManipulator
: public WorldManipulator {

public:
	explicit InteractiveManipulator(Audio &, const SoundBank &, Entity &);

	void SetBlock(Chunk &, int, const Block &) override;

private:
	Entity &player;
	Audio &audio;
	const SoundBank &sounds;

};

}

#endif
