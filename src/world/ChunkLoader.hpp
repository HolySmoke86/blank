#ifndef BLANK_WORLD_CHUNKLOADER_HPP_
#define BLANK_WORLD_CHUNKLOADER_HPP_

#include <list>


namespace blank {

class ChunkStore;
class Generator;
class WorldSave;

class ChunkLoader {

public:
	ChunkLoader(
		ChunkStore &,
		const Generator &,
		const WorldSave &
	) noexcept;

	const WorldSave &SaveFile() const noexcept { return save; }

	void Update(int dt);

	int ToLoad() const noexcept;

	// returns true if the chunk was generated
	// (as opposed to loaded from file)
	bool LoadOne();
	void LoadN(std::size_t n);

private:
	ChunkStore &store;
	const Generator &gen;
	const WorldSave &save;

};

}

#endif
