#ifndef BLANK_STANDALONE_PRELOADSTATE_HPP_
#define BLANK_STANDALONE_PRELOADSTATE_HPP_

#include "../app/State.hpp"

#include "../ui/Progress.hpp"

#include <cstddef>


namespace blank {

class ChunkLoader;
class ChunkRenderer;
class Environment;

namespace standalone {

class PreloadState
: public State {

public:
	PreloadState(Environment &, ChunkLoader &, ChunkRenderer &);

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

private:
	Environment &env;
	ChunkLoader &loader;
	ChunkRenderer &render;
	Progress progress;
	std::size_t total;
	std::size_t per_update;

};

}
}

#endif
