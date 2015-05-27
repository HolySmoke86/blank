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

	static constexpr bool InBounds(const Block::Pos &pos) {
		return
			pos.x >= 0 && pos.x < Width() &&
			pos.y >= 0 && pos.y < Height() &&
			pos.z >= 0 && pos.z < Depth();
	}
	static constexpr bool InBounds(const Pos &pos) {
		return
			pos.x >= 0 && pos.x < Width() &&
			pos.y >= 0 && pos.y < Height() &&
			pos.z >= 0 && pos.z < Depth();
	}
	static constexpr int ToIndex(const Pos &pos) {
		return pos.x + pos.y * Width() + pos.z * Width() * Height();
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
	static Pos ToPos(int idx) {
		return Pos(
			(idx % Width()),
			((idx / Width()) % Height()),
			(idx / (Width() * Height()))
		);
	}
	glm::mat4 ToTransform(int idx) const;

	static constexpr bool IsBorder(int idx) {
		return
			idx < Width() * Height() ||                    // low Z plane
			idx % Width() == 0 ||                          // low X plane
			(idx / (Width() * Height())) == Depth() - 1 || // high Z plane
			idx % Width() == Width() - 1 ||                // high X plane
			(idx / Width()) % Height() == 0 ||             // low Y plane
			(idx / Width()) % Height() == Height() - 1;    // high Y plane
	}

	bool IsSurface(int index) const { return IsSurface(ToPos(index)); }
	bool IsSurface(const Block::Pos &pos) const { return IsSurface(Pos(pos)); }
	bool IsSurface(const Pos &pos) const;

	void SetNeighbor(Chunk &);
	bool HasNeighbor(Block::Face f) const { return neighbor[f]; }
	Chunk &GetNeighbor(Block::Face f) { return *neighbor[f]; }
	const Chunk &GetNeighbor(Block::Face f) const { return *neighbor[f]; }
	void ClearNeighbors();
	void Unlink();
	void Relink();

	// check which faces of a block at given index are obstructed (and therefore invisible)
	Block::FaceSet Obstructed(int idx) const;

	void Invalidate() { dirty = true; }

	void SetBlock(int index, const Block &);
	void SetBlock(const Block::Pos &pos, const Block &block) { SetBlock(ToIndex(pos), block); }
	void SetBlock(const Pos &pos, const Block &block) { SetBlock(ToIndex(pos), block); }

	const Block &BlockAt(int index) const { return blocks[index]; }
	const Block &BlockAt(const Block::Pos &pos) const { return BlockAt(ToIndex(pos)); }
	const Block &BlockAt(const Pos &pos) const { return BlockAt(ToIndex(pos)); }

	const BlockType &Type(const Block &b) const { return *types->Get(b.type); }

	void SetLight(int index, int level);
	void SetLight(const Pos &pos, int level) { SetLight(ToIndex(pos), level); }
	void SetLight(const Block::Pos &pos, int level) { SetLight(ToIndex(pos), level); }

	int GetLight(int index) const;
	int GetLight(const Pos &pos) const { return GetLight(ToIndex(pos)); }
	int GetLight(const Block::Pos &pos) const { return GetLight(ToIndex(pos)); }

	float GetVertexLight(int index, const BlockModel::Position &, const Model::Normal &) const;

	bool Intersection(
		const Ray &ray,
		const glm::mat4 &M,
		float &dist
	) const {
		return blank::Intersection(ray, Bounds(), M, &dist);
	}

	bool Intersection(
		const Ray &,
		const glm::mat4 &M,
		int &blkid,
		float &dist,
		glm::vec3 &normal) const;

	void Position(const Pos &);
	const Pos &Position() const { return position; }
	glm::mat4 Transform(const Pos &offset) const;

	void CheckUpdate();
	void Draw();

private:
	void Update();

private:
	const BlockTypeRegistry *types;
	Chunk *neighbor[Block::FACE_COUNT];
	Block blocks[16 * 16 * 16];
	unsigned char light[16 * 16 * 16];
	BlockModel model;
	Pos position;
	bool dirty;

};


class BlockLookup {

public:
	// resolve chunk/position from oob coordinates
	BlockLookup(Chunk *c, const Chunk::Pos &p);

	// resolve chunk/position from ib coordinates and direction
	BlockLookup(Chunk *c, const Chunk::Pos &p, Block::Face dir);

	// check if lookup was successful
	operator bool() const { return chunk; }

	// only valid if lookup was successful
	Chunk &GetChunk() const { return *chunk; }
	const Chunk::Pos &GetBlockPos() const { return pos; }
	const Block &GetBlock() const { return GetChunk().BlockAt(GetBlockPos()); }
	const BlockType &GetType() const { return GetChunk().Type(GetBlock()); }
	int GetLight() const { return GetChunk().GetLight(GetBlockPos()); }

private:
	Chunk *chunk;
	Chunk::Pos pos;

};


class Generator;

class ChunkLoader {

public:
	struct Config {
		int load_dist = 6;
		int unload_dist = 8;
	};

	ChunkLoader(const Config &, const BlockTypeRegistry &, const Generator &);

	void Generate(const Chunk::Pos &from, const Chunk::Pos &to);
	void GenerateSurrounding(const Chunk::Pos &);

	std::list<Chunk> &Loaded() { return loaded; }

	Chunk *Loaded(const Chunk::Pos &);
	bool Queued(const Chunk::Pos &);
	bool Known(const Chunk::Pos &);
	Chunk &ForceLoad(const Chunk::Pos &);

	void Rebase(const Chunk::Pos &);
	void Update();

private:
	Chunk &Generate(const Chunk::Pos &pos);
	void Insert(Chunk &);
	void Remove(Chunk &);

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
