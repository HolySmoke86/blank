#ifndef BLANK_CLIENT_CHUNKREQUESTER_HPP_
#define BLANK_CLIENT_CHUNKREQUESTER_HPP_

#include <cstddef>


namespace blank {

class ChunkStore;
class WorldSave;

namespace client {

class ChunkRequester {

public:
	ChunkRequester(
		ChunkStore &,
		const WorldSave &
	) noexcept;

	const WorldSave &SaveFile() const noexcept { return save; }

	void Update(int dt);

	int ToLoad() const noexcept;

	void LoadOne();
	void LoadN(std::size_t n);

private:
	ChunkStore &store;
	const WorldSave &save;

};

}
}

#endif
