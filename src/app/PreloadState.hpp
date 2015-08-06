#ifndef BLANK_APP_PRELOADSTATE_HPP_
#define BLANK_APP_PRELOADSTATE_HPP_

#include "State.hpp"

#include <cstddef>


namespace blank {

class ChunkLoader;
class Environment;

class PreloadState
: public State {

public:
	PreloadState(Environment &, ChunkLoader &);

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

private:
	Environment &env;
	ChunkLoader &loader;
	std::size_t per_update;

};

}

#endif
