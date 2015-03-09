#ifndef BLANK_CHUNK_HPP_
#define BLANK_CHUNK_HPP_

#include "block.hpp"
#include "geometry.hpp"
#include "model.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blank {

/// cube of size 16 (256 tiles, 4096 blocks)
class Chunk {

public:
	Chunk();

	Chunk(Chunk &&);
	Chunk &operator =(Chunk &&);

	static constexpr int Width() { return 16; }
	static constexpr int Height() { return 16; }
	static constexpr int Depth() { return 16; }
	static glm::tvec3<int> Extent() { return { Width(), Height(), Depth() }; }
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
	static glm::vec3 ToCoords(int idx) {
		return glm::vec3(
			0.5f + idx % Width(),
			0.5f + (idx / Width()) % Height(),
			0.5f + idx / (Width() * Height())
		);
	}

	void Allocate();
	void Invalidate() { dirty = true; }

	Block &BlockAt(int index) { return blocks[index]; }
	const Block &BlockAt(int index) const { return blocks[index]; }
	Block &BlockAt(const glm::vec3 &pos) { return BlockAt(ToIndex(pos)); }
	const Block &BlockAt(const glm::vec3 &pos) const { return BlockAt(ToIndex(pos)); }

	bool Intersection(
		const Ray &,
		const glm::mat4 &M,
		int *blkid = nullptr,
		float *dist = nullptr,
		glm::vec3 *normal = nullptr) const;

	void Position(const glm::tvec3<int> &);
	const glm::tvec3<int> &Position() const { return position; }
	glm::mat4 Transform(const glm::tvec3<int> &offset) const;

	void Draw();

private:
	int VertexCount() const;
	void Update();

private:
	std::vector<Block> blocks;
	Model model;
	glm::tvec3<int> position;
	bool dirty;

};

}

#endif
