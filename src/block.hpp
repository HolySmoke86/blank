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

	static const NullShape DEFAULT_SHAPE;


	void FillModel(
		Model &m,
		const glm::vec3 &pos_offset = { 0, 0, 0 },
		Model::Index idx_offset = 0
	) const;
	void FillOutlineModel(
		OutlineModel &m,
		const glm::vec3 &pos_offset = { 0, 0, 0 },
		OutlineModel::Index idx_offset = 0
	) const;

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

	int type;

	constexpr explicit Block(int type = 0)
	: type(type) { }

};

}

#endif
