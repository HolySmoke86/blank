#ifndef BLANK_CHUNK_HPP_
#define BLANK_CHUNK_HPP_

#include "block.hpp"
#include "geometry.hpp"
#include "model.hpp"

#include <list>
#include <vector>
#include <glm/glm.hpp>


namespace blank {

/// cube of size 16 (256 tiles, 4096 blocks)
class Chunk {

public:
	using Pos = glm::tvec3<int>;

public:
	explicit Chunk(const BlockTypeRegistry &);

	Chunk(Chunk &&);
	Chunk &operator =(Chunk &&);

	static constexpr int Width() { return 16; }
	static constexpr int Height() { return 16; }
	static constexpr int Depth() { return 16; }
	static Pos Extent() { return { Width(), Height(), Depth() }; }
	static constexpr int Size() { return Width() * Height() * Depth(); }

	static AABB Bounds() { return AABB{ { 0, 0, 0 }, Extent() }; }

	static constexpr bool InBounds(const glm::vec3 &pos) {
		return
			pos.x >= 0 && pos.x < Width() &&
			pos.y >= 0 && pos.y < Height() &&
			pos.z >= 0 && pos.z < Depth();
	}
	static constexpr int ToIndex(const glm::vec3 &pos) {
		return int(pos.x) + int(pos.y) * Width() + int(pos.z) * Width() * Height();
	}
	static constexpr bool InBounds(int idx) {
		return idx >= 0 && idx < Size();
	}
	static Block::Pos ToCoords(int idx) {
		return Block::Pos(
			0.5f + (idx % Width()),
			0.5f + ((idx / Width()) % Height()),
			0.5f + (idx / (Width() * Height()))
		);
	}

	static constexpr bool IsBorder(int idx) {
		return
			idx < Width() * Height() ||
			(idx / Width()) % Height() == 0 ||
			(idx / Width()) % Height() == Height() - 1 ||
			(idx / (Width() * Height())) == Depth() - 1;
	}

	// check if block at given index is completely enclosed (and therefore invisible)
	bool Obstructed(int idx) const;

	void Allocate();
	void Invalidate() { dirty = true; }

	Block &BlockAt(int index) { return blocks[index]; }
	const Block &BlockAt(int index) const { return blocks[index]; }
	Block &BlockAt(const Block::Pos &pos) { return BlockAt(ToIndex(pos)); }
	const Block &BlockAt(const Block::Pos &pos) const { return BlockAt(ToIndex(pos)); }

	const BlockType &Type(const Block &b) const { return *types->Get(b.type); }

	bool Intersection(
		const Ray &,
		const glm::mat4 &M,
		int *blkid = nullptr,
		float *dist = nullptr,
		glm::vec3 *normal = nullptr) const;

	void Position(const Pos &);
	const Pos &Position() const { return position; }
	glm::mat4 Transform(const Pos &offset) const;

	void CheckUpdate();
	void Draw();

private:
	void Update();

private:
	const BlockTypeRegistry *types;
	std::vector<Block> blocks;
	Model model;
	Pos position;
	bool dirty;

};


class Generator;

class ChunkLoader {

public:
	ChunkLoader(const BlockTypeRegistry &, const Generator &);

	void Generate(const Chunk::Pos &from, const Chunk::Pos &to);

	std::list<Chunk> &Loaded() { return loaded; }

	Chunk *Loaded(const Chunk::Pos &);
	bool Queued(const Chunk::Pos &);
	bool Known(const Chunk::Pos &);
	Chunk &ForceLoad(const Chunk::Pos &);

	void Rebase(const Chunk::Pos &);
	void Update();

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
