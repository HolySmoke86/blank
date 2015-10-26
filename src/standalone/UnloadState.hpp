#ifndef BLANK_STANDALONE_UNLOADSTATE_HPP_
#define BLANK_STANDALONE_UNLOADSTATE_HPP_

#include "../shared/ProgressState.hpp"

#include <cstddef>
#include <list>


namespace blank {

class Chunk;
class ChunkStore;
class Environment;
class WorldSave;

namespace standalone {

class UnloadState
: public ProgressState {

public:
	UnloadState(Environment &, ChunkStore &, const WorldSave &);

	void OnResume();

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;

private:
	Environment &env;
	ChunkStore &chunks;
	const WorldSave &save;
	std::list<Chunk>::iterator cur;
	std::list<Chunk>::iterator end;
	std::size_t done;
	std::size_t total;
	std::size_t per_update;

};

}
}

#endif
