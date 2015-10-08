#ifndef BLANK_STANDALONE_PRELOADSTATE_HPP_
#define BLANK_STANDALONE_PRELOADSTATE_HPP_

#include "../app/ProgressState.hpp"

#include <cstddef>


namespace blank {

class ChunkLoader;
class ChunkRenderer;
class Environment;

namespace standalone {

class PreloadState
: public ProgressState {

public:
	PreloadState(Environment &, ChunkLoader &, ChunkRenderer &);

	void Update(int dt) override;

private:
	Environment &env;
	ChunkLoader &loader;
	ChunkRenderer &render;
	std::size_t total;
	std::size_t per_update;

};

}
}

#endif
