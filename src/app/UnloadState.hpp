#ifndef BLANK_APP_UNLOADSTATE_HPP_
#define BLANK_APP_UNLOADSTATE_HPP_

#include "State.hpp"

#include "../ui/Progress.hpp"

#include <cstddef>
#include <list>


namespace blank {

class Chunk;
class ChunkLoader;
class Environment;

class UnloadState
: public State {

public:
	UnloadState(Environment &, ChunkLoader &);

	void OnResume();

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

private:
	Environment &env;
	ChunkLoader &loader;
	Progress progress;
	std::list<Chunk>::iterator cur;
	std::list<Chunk>::iterator end;
	std::size_t done;
	std::size_t total;
	std::size_t per_update;

};

}

#endif
