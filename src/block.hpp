#ifndef BLANK_BLOCK_HPP_
#define BLANK_BLOCK_HPP_

#include "geometry.hpp"
#include "model.hpp"
#include "shape.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blank {

/// attributes of a type of block
struct BlockType {

	int id;

	bool visible;

	const Shape *shape;
	glm::vec3 color;
	glm::vec3 outline_color;

	explicit BlockType(
		bool v = false,
		const glm::vec3 &color = { 1, 1, 1 },
		const Shape *shape = &DEFAULT_SHAPE,
		const glm::vec3 &outline_color = { -1, -1, -1 })
	: id(-1), visible(v), shape(shape), color(color), outline_color(outline_color) { }

	static const BlockType DEFAULT;
	static const NullShape DEFAULT_SHAPE;


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

	size_t Size() const { return types.size(); }

	BlockType *operator [](int id) { return &types[id]; }
	const BlockType *Get(int id) const { return &types[id]; }

private:
	std::vector<BlockType> types;

};


/// single 1x1x1 cube
struct Block {

	using Pos = glm::vec3;

	const BlockType *type;

	constexpr explicit Block(const BlockType *t = &BlockType::DEFAULT)
	: type(t) { }

};

}

#endif
