#ifndef BLANK_WORLD_CHUNKLOADER_HPP_
#define BLANK_WORLD_CHUNKLOADER_HPP_

#include "Chunk.hpp"

#include <list>


namespace blank {

class BlockTypeRegistry;
class Generator;

class ChunkLoader {

public:
	struct Config {
		int load_dist = 6;
		int unload_dist = 8;
	};

	ChunkLoader(const Config &, const BlockTypeRegistry &, const Generator &) noexcept;

	void Generate(const Chunk::Pos &from, const Chunk::Pos &to);
	void GenerateSurrounding(const Chunk::Pos &);

	std::list<Chunk> &Loaded() noexcept { return loaded; }

	Chunk *Loaded(const Chunk::Pos &) noexcept;
	bool Queued(const Chunk::Pos &) noexcept;
	bool Known(const Chunk::Pos &) noexcept;
	Chunk &ForceLoad(const Chunk::Pos &);

	void Rebase(const Chunk::Pos &);
	void Update();

private:
	Chunk &Generate(const Chunk::Pos &pos);
	void Insert(Chunk &) noexcept;
	void Remove(Chunk &) noexcept;

private:
	Chunk::Pos base;

	const BlockTypeRegistry &reg;
	const Generator &gen;

	std::list<Chunk> loaded;
	std::list<Chunk::Pos> to_generate;
	std::list<Chunk> to_free;

	int load_dist;
	int unload_dist;

};

}

#endif
