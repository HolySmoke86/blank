#ifndef BLANK_WORLD_CHUNK_HPP_
#define BLANK_WORLD_CHUNK_HPP_

#include "Block.hpp"
#include "BlockTypeRegistry.hpp"
#include "../geometry/Location.hpp"
#include "../geometry/primitive.hpp"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


namespace blank {

class BlockType;
class WorldCollision;

/// cube of size 16 (256 tiles, 4096 blocks)
class Chunk {

public:
	explicit Chunk(const BlockTypeRegistry &) noexcept;

	Chunk(Chunk &&) noexcept;
	Chunk &operator =(Chunk &&) noexcept;

	static constexpr int side = ExactLocation::scale;
	static constexpr float fside = ExactLocation::fscale;
	static constexpr int size = side * side * side;

	static AABB Bounds() noexcept { return AABB{ { 0.0f, 0.0f, 0.0f }, ExactLocation::FExtent() }; }

	static constexpr bool InBounds(const ExactLocation::Fine &pos) noexcept {
		return
			pos.x >= 0.0f && pos.x < fside &&
			pos.y >= 0.0f && pos.y < fside &&
			pos.z >= 0.0f && pos.z < fside;
	}
	static constexpr bool InBounds(const RoughLocation::Fine &pos) noexcept {
		return
			pos.x >= 0 && pos.x < side &&
			pos.y >= 0 && pos.y < side &&
			pos.z >= 0 && pos.z < side;
	}
	static constexpr int ToIndex(const RoughLocation::Fine &pos) noexcept {
		return pos.x + pos.y * side + pos.z * side * side;
	}
	static constexpr bool InBounds(int idx) noexcept {
		return idx >= 0 && idx < size;
	}
	static ExactLocation::Fine ToCoords(int idx) noexcept {
		return ExactLocation::Fine(
			0.5f + (idx % side),
			0.5f + ((idx / side) % side),
			0.5f + (idx / (side * side))
		);
	}
	static ExactLocation::Fine ToCoords(const RoughLocation::Fine &pos) noexcept {
		return ExactLocation::Fine(pos) + 0.5f;
	}
	static RoughLocation::Fine ToPos(int idx) noexcept {
		return RoughLocation::Fine(
			(idx % side),
			((idx / side) % side),
			(idx / (side * side))
		);
	}
	glm::mat4 ToTransform(const RoughLocation::Fine &pos, int idx) const noexcept;

	ExactLocation::Fine ToSceneCoords(const ExactLocation::Coarse &base, const ExactLocation::Fine &pos) const noexcept {
		return ExactLocation::Fine((position - base) * ExactLocation::Extent()) + pos;
	}

	static bool IsBorder(const RoughLocation::Fine &pos) noexcept {
		return
			pos.x == 0 ||
			pos.x == side - 1 ||
			pos.y == 0 ||
			pos.y == side - 1 ||
			pos.z == 0 ||
			pos.z == side - 1;
	}
	static constexpr bool IsBorder(int idx) noexcept {
		return
			idx < side * side ||                 // low Z plane
			idx % side == 0 ||                   // low X plane
			(idx / (side * side)) == side - 1 || // high Z plane
			idx % side == side - 1 ||            // high X plane
			(idx / side) % side == 0 ||          // low Y plane
			(idx / side) % side == side - 1;     // high Y plane
	}

	bool IsSurface(int index) const noexcept { return IsSurface(ToPos(index)); }
	bool IsSurface(const ExactLocation::Fine &pos) const noexcept { return IsSurface(RoughLocation::Fine(pos)); }
	bool IsSurface(const RoughLocation::Fine &pos) const noexcept;

	void SetNeighbor(Block::Face, Chunk &) noexcept;
	bool HasNeighbor(Block::Face f) const noexcept { return neighbor[f]; }
	Chunk &GetNeighbor(Block::Face f) noexcept { return *neighbor[f]; }
	const Chunk &GetNeighbor(Block::Face f) const noexcept { return *neighbor[f]; }
	void Unlink() noexcept;

	// check which faces of a block at given index are obstructed (and therefore invisible)
	Block::FaceSet Obstructed(const RoughLocation::Fine &) const noexcept;

	void SetBlock(int index, const Block &) noexcept;
	void SetBlock(const ExactLocation::Fine &pos, const Block &block) noexcept { SetBlock(ToIndex(pos), block); }
	void SetBlock(const RoughLocation::Fine &pos, const Block &block) noexcept { SetBlock(ToIndex(pos), block); }

	const Block &BlockAt(int index) const noexcept { return blocks[index]; }
	const Block &BlockAt(const ExactLocation::Fine &pos) const noexcept { return BlockAt(ToIndex(pos)); }
	const Block &BlockAt(const RoughLocation::Fine &pos) const noexcept { return BlockAt(ToIndex(pos)); }

	const BlockType &Type(const Block &b) const noexcept { return types->Get(b.type); }
	const BlockType &Type(int index) const noexcept { return Type(BlockAt(index)); }

	void SetLight(int index, int level) noexcept;
	void SetLight(const ExactLocation::Fine &pos, int level) noexcept { SetLight(ToIndex(pos), level); }
	void SetLight(const RoughLocation::Fine &pos, int level) noexcept { SetLight(ToIndex(pos), level); }

	int GetLight(int index) const noexcept;
	int GetLight(const ExactLocation::Fine &pos) const noexcept { return GetLight(ToIndex(pos)); }
	int GetLight(const RoughLocation::Fine &pos) const noexcept { return GetLight(ToIndex(pos)); }

	float GetVertexLight(const RoughLocation::Fine &, const BlockMesh::Position &, const EntityMesh::Normal &) const noexcept;

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
		WorldCollision &) noexcept;

	bool Intersection(
		const AABB &box,
		const glm::mat4 &Mbox,
		const glm::mat4 &Mchunk,
		std::vector<WorldCollision> &) noexcept;

	void Position(const ExactLocation::Coarse &pos) noexcept { position = pos; }
	const ExactLocation::Coarse &Position() const noexcept { return position; }
	glm::mat4 Transform(const ExactLocation::Coarse &offset) const noexcept {
		return glm::translate((position - offset) * ExactLocation::Extent());
	}

	void *BlockData() noexcept { return &blocks[0]; }
	const void *BlockData() const noexcept { return &blocks[0]; }
	static constexpr std::size_t BlockSize() noexcept { return offsetof(Chunk, position) - offsetof(Chunk, blocks); }

	bool Generated() const noexcept { return generated; }
	void SetGenerated() noexcept { generated = true; }
	bool Lighted() const noexcept { return lighted; }
	void ScanLights();

	void Ref() noexcept { ++ref_count; }
	void UnRef() noexcept { --ref_count; }
	bool Referenced() const noexcept { return ref_count > 0; }

	void Invalidate() noexcept { dirty_mesh = dirty_save = true; }
	void InvalidateMesh() noexcept { dirty_mesh = true; }
	void ClearMesh() noexcept { dirty_mesh = false; }
	void ClearSave() noexcept { dirty_save = false; }
	bool ShouldUpdateMesh() const noexcept { return dirty_mesh; }
	bool ShouldUpdateSave() const noexcept { return dirty_save; }

	void Update(BlockMesh &) noexcept;

private:
	const BlockTypeRegistry *types;
	Chunk *neighbor[Block::FACE_COUNT];

	Block blocks[size];
	unsigned char light[size];
	bool generated;
	bool lighted;

	ExactLocation::Coarse position;
	int ref_count;
	bool dirty_mesh;
	bool dirty_save;

};

}

#endif
