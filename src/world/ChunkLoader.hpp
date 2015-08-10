#ifndef BLANK_WORLD_CHUNKLOADER_HPP_
#define BLANK_WORLD_CHUNKLOADER_HPP_

#include "Chunk.hpp"
#include "../app/IntervalTimer.hpp"

#include <list>


namespace blank {

class BlockTypeRegistry;
class Generator;
class WorldSave;

class ChunkLoader {

public:
	struct Config {
		int load_dist = 6;
		int unload_dist = 8;
		int gen_limit = 16;
	};

	ChunkLoader(
		const Config &,
		const BlockTypeRegistry &,
		const Generator &,
		const WorldSave &
	) noexcept;

	void Queue(const Chunk::Pos &from, const Chunk::Pos &to);
	void QueueSurrounding(const Chunk::Pos &);

	std::list<Chunk> &Loaded() noexcept { return loaded; }

	Chunk *Loaded(const Chunk::Pos &) noexcept;
	bool Queued(const Chunk::Pos &) noexcept;
	bool Known(const Chunk::Pos &) noexcept;
	Chunk &ForceLoad(const Chunk::Pos &);

	bool OutOfRange(const Chunk &c) const noexcept { return OutOfRange(c.Position()); }
	bool OutOfRange(const Chunk::Pos &) const noexcept;

	void Rebase(const Chunk::Pos &);
	void Update(int dt);

	std::size_t ToLoad() const noexcept { return to_load.size(); }
	// returns true if the chunk was generated
	bool LoadOne();
	void LoadN(std::size_t n);

private:
	Chunk &Load(const Chunk::Pos &pos);
	// link given chunk to all loaded neighbors
	void Insert(Chunk &) noexcept;
	// remove a loaded chunk
	// this unlinks it from its neighbors as well as moves it to the free list
	// given iterator must point to a chunk from the loaded list
	// returns an iterator to the chunk following the removed one
	// in the loaded list (end for the last one)
	std::list<Chunk>::iterator Remove(std::list<Chunk>::iterator) noexcept;

private:
	Chunk::Pos base;

	const BlockTypeRegistry &reg;
	const Generator &gen;
	const WorldSave &save;

	std::list<Chunk> loaded;
	std::list<Chunk::Pos> to_load;
	std::list<Chunk> to_free;

	IntervalTimer gen_timer;

	int load_dist;
	int unload_dist;

};

}

#endif
