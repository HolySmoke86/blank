#ifndef BLANK_WORLD_HPP_
#define BLANK_WORLD_HPP_

#include "model.hpp"
#include "geometry.hpp"

#include <list>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

/// attributes of a type of block
struct BlockType {

	int id;

	bool visible;
	glm::vec3 color;
	glm::vec3 outline_color;

	constexpr explicit BlockType(
		bool v = false,
		const glm::vec3 &color = { 1, 1, 1 },
		const glm::vec3 &outline_color = { -1, -1, -1 })
	: id(-1), visible(v), color(color), outline_color(outline_color) { }

	static const BlockType DEFAULT;


	void FillVBO(
		const glm::vec3 &pos,
		std::vector<glm::vec3> &vertices,
		std::vector<glm::vec3> &colors,
		std::vector<glm::vec3> &normals
	) const;

	void FillModel(const glm::vec3 &pos, Model &m) const {
		FillVBO(pos, m.vertices, m.colors, m.normals);
		m.Invalidate();
	}


	void FillOutlineVBO(
		std::vector<glm::vec3> &vertices,
		std::vector<glm::vec3> &colors
	) const;

	void FillOutlineModel(OutlineModel &m) const {
		FillOutlineVBO(m.vertices, m.colors);
		m.Invalidate();
	}

};


class BlockTypeRegistry {

public:
	BlockTypeRegistry();

public:
	int Add(const BlockType &);

	BlockType *operator [](int id) { return &types[id]; }
	const BlockType *Get(int id) const { return &types[id]; }

private:
	std::vector<BlockType> types;

};


/// single 1x1x1 cube
struct Block {

	const BlockType *type;

	constexpr explicit Block(const BlockType *t = &BlockType::DEFAULT)
	: type(t) { }

};


/// cube of size 16 (256 tiles, 4096 blocks)
class Chunk {

public:
	Chunk();

	Chunk(Chunk &&);
	Chunk &operator =(Chunk &&);

	static constexpr int Width() { return 16; }
	static constexpr int Height() { return 16; }
	static constexpr int Depth() { return 16; }
	static glm::vec3 Extent() { return glm::vec3(Width(), Height(), Depth()); }
	static constexpr int Size() { return Width() * Height() * Depth(); }

	static constexpr bool InBounds(const glm::vec3 &pos) {
		return
			pos.x >= 0 && pos.x < Width() &&
			pos.y >= 0 && pos.y < Height() &&
			pos.z >= 0 && pos.z < Depth();
	}
	static constexpr int ToIndex(const glm::vec3 &pos) {
		return pos.x + pos.y * Width() + pos.z * Width() * Height();
	}
	static constexpr bool InBounds(int idx) {
		return idx >= 0 && idx < Size();
	}
	static glm::vec3 ToCoords(int idx) {
		return glm::vec3(
			idx % Width(),
			(idx / Width()) % Height(),
			idx / (Width() * Height())
		);
	}

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

	void Position(const glm::vec3 &);
	const glm::vec3 &Position() const { return position; }
	const glm::mat4 &Transform() const { return transform; }

	void Draw();

private:
	int VertexCount() const;
	void Update();

private:
	std::vector<Block> blocks;
	Model model;
	glm::vec3 position;
	glm::mat4 transform;
	bool dirty;

};


class World {

public:
	World();

	void Generate();

	bool Intersection(
		const Ray &,
		const glm::mat4 &M,
		Chunk **chunk = nullptr,
		int *blkid = nullptr,
		float *dist = nullptr,
		glm::vec3 *normal = nullptr);

	BlockTypeRegistry &BlockTypes() { return blockType; }
	std::list<Chunk> &LoadedChunks() { return chunks; }

	Chunk &Next(const Chunk &, const glm::vec3 &dir);

private:
	Chunk &Generate(const glm::vec3 &);

private:
	BlockTypeRegistry blockType;
	std::list<Chunk> chunks;

};

}

#endif
