#ifndef BLANK_WORLD_HPP_
#define BLANK_WORLD_HPP_

#include "model.hpp"

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

/// attributes of a type of block
struct BlockType {

	bool visible;

	constexpr explicit BlockType(bool v = false)
	: visible(v) { }

	static const BlockType DEFAULT;


	void FillVBO(
		const glm::vec3 &pos,
		std::vector<glm::vec3> &vertices,
		std::vector<glm::vec3> &colors,
		std::vector<glm::vec3> &normals
	) const;

	void FillModel(const glm::vec3 &pos, Model &m) const {
		FillVBO(pos, m.vertices, m.colors, m.normals);
	}

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

	static constexpr int Width() { return 16; }
	static constexpr int Height() { return 16; }
	static constexpr int Depth() { return 16; }
	static constexpr int Size() { return Width() * Height() * Depth(); }

	static constexpr int ToIndex(const glm::vec3 &pos) {
		return pos.x + pos.y * Width() + pos.z * Width() * Height();
	}
	static glm::vec3 ToCoords(int idx) {
		return glm::vec3(
			idx % Width(),
			(idx / Width()) % Height(),
			idx / (Width() * Height())
		);
	}

	void Invalidate() { dirty = true; }

	Block &BlockAt(const glm::vec3 &pos) { return blocks[ToIndex(pos)]; }
	const Block &BlockAt(const glm::vec3 &pos) const { return blocks[ToIndex(pos)]; }

	void Draw();

private:
	int VertexCount() const;
	void Update();

private:
	std::vector<Block> blocks;
	Model model;
	bool dirty;

};

}

#endif
