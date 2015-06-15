#ifndef BLANK_WORLD_CHUNK_HPP_
#define BLANK_WORLD_CHUNK_HPP_

#include "Block.hpp"
#include "BlockTypeRegistry.hpp"
#include "../model/BlockModel.hpp"
#include "../model/geometry.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


namespace blank {

class BlockType;

/// cube of size 16 (256 tiles, 4096 blocks)
class Chunk {

public:
	using Pos = glm::tvec3<int>;

public:
	explicit Chunk(const BlockTypeRegistry &) noexcept;

	Chunk(Chunk &&) noexcept;
	Chunk &operator =(Chunk &&) noexcept;

	static constexpr int width = 16;
	static constexpr int height = 16;
	static constexpr int depth = 16;
	static Pos Extent() noexcept { return { width, height, depth }; }
	static constexpr int size = width * height * depth;

	static AABB Bounds() noexcept { return AABB{ { 0, 0, 0 }, Extent() }; }

	static constexpr bool InBounds(const Block::Pos &pos) noexcept {
		return
			pos.x >= 0 && pos.x < width &&
			pos.y >= 0 && pos.y < height &&
			pos.z >= 0 && pos.z < depth;
	}
	static constexpr bool InBounds(const Pos &pos) noexcept {
		return
			pos.x >= 0 && pos.x < width &&
			pos.y >= 0 && pos.y < height &&
			pos.z >= 0 && pos.z < depth;
	}
	static constexpr int ToIndex(const Pos &pos) noexcept {
		return pos.x + pos.y * width + pos.z * width * height;
	}
	static constexpr bool InBounds(int idx) noexcept {
		return idx >= 0 && idx < size;
	}
	static Block::Pos ToCoords(int idx) noexcept {
		return Block::Pos(
			0.5f + (idx % width),
			0.5f + ((idx / width) % height),
			0.5f + (idx / (width * height))
		);
	}
	static Block::Pos ToCoords(const Pos &pos) noexcept {
		return Block::Pos(pos) + 0.5f;
	}
	static Pos ToPos(int idx) noexcept {
		return Pos(
			(idx % width),
			((idx / width) % height),
			(idx / (width * height))
		);
	}
	glm::mat4 ToTransform(const Pos &pos, int idx) const noexcept;

	static bool IsBorder(const Pos &pos) noexcept {
		return
			pos.x == 0 ||
			pos.x == width - 1 ||
			pos.y == 0 ||
			pos.y == height - 1 ||
			pos.z == 0 ||
			pos.z == depth - 1;
	}
	static constexpr bool IsBorder(int idx) noexcept {
		return
			idx < width * height ||                    // low Z plane
			idx % width == 0 ||                          // low X plane
			(idx / (width * height)) == depth - 1 || // high Z plane
			idx % width == width - 1 ||                // high X plane
			(idx / width) % height == 0 ||             // low Y plane
			(idx / width) % height == height - 1;    // high Y plane
	}

	bool IsSurface(int index) const noexcept { return IsSurface(ToPos(index)); }
	bool IsSurface(const Block::Pos &pos) const noexcept { return IsSurface(Pos(pos)); }
	bool IsSurface(const Pos &pos) const noexcept;

	void SetNeighbor(Chunk &) noexcept;
	bool HasNeighbor(Block::Face f) const noexcept { return neighbor[f]; }
	Chunk &GetNeighbor(Block::Face f) noexcept { return *neighbor[f]; }
	const Chunk &GetNeighbor(Block::Face f) const noexcept { return *neighbor[f]; }
	void ClearNeighbors() noexcept;
	void Unlink() noexcept;
	void Relink() noexcept;

	// check which faces of a block at given index are obstructed (and therefore invisible)
	Block::FaceSet Obstructed(const Pos &) const noexcept;

	void Invalidate() noexcept { dirty = true; }

	void SetBlock(int index, const Block &) noexcept;
	void SetBlock(const Block::Pos &pos, const Block &block) noexcept { SetBlock(ToIndex(pos), block); }
	void SetBlock(const Pos &pos, const Block &block) noexcept { SetBlock(ToIndex(pos), block); }

	const Block &BlockAt(int index) const noexcept { return blocks[index]; }
	const Block &BlockAt(const Block::Pos &pos) const noexcept { return BlockAt(ToIndex(pos)); }
	const Block &BlockAt(const Pos &pos) const noexcept { return BlockAt(ToIndex(pos)); }

	const BlockType &Type(const Block &b) const noexcept { return types->Get(b.type); }
	const BlockType &Type(int index) const noexcept { return Type(BlockAt(index)); }

	void SetLight(int index, int level) noexcept;
	void SetLight(const Pos &pos, int level) noexcept { SetLight(ToIndex(pos), level); }
	void SetLight(const Block::Pos &pos, int level) noexcept { SetLight(ToIndex(pos), level); }

	int GetLight(int index) const noexcept;
	int GetLight(const Pos &pos) const noexcept { return GetLight(ToIndex(pos)); }
	int GetLight(const Block::Pos &pos) const noexcept { return GetLight(ToIndex(pos)); }

	float GetVertexLight(const Pos &, const BlockModel::Position &, const Model::Normal &) const noexcept;

	bool Intersection(
		const Ray &ray,
		const glm::mat4 &M,
		float &dist
	) const noexcept {
		return blank::Intersection(ray, Bounds(), M, &dist);
	}

	bool Intersection(
		const Ray &,
		const glm::mat4 &M,
		int &blkid,
		float &dist,
		glm::vec3 &normal) const noexcept;

	void Position(const Pos &pos) noexcept { position = pos; }
	const Pos &Position() const noexcept { return position; }
	glm::mat4 Transform(const Pos &offset) const noexcept {
		return glm::translate((position - offset) * Extent());
	}

	void CheckUpdate() noexcept;
	void Draw() noexcept;

private:
	void Update() noexcept;

private:
	const BlockTypeRegistry *types;
	Chunk *neighbor[Block::FACE_COUNT];
	Block blocks[size];
	unsigned char light[size];
	BlockModel model;
	Pos position;
	bool dirty;

};

}

#endif
