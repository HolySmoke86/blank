#ifndef BLANK_WORLD_BLOCKTYPE_HPP_
#define BLANK_WORLD_BLOCKTYPE_HPP_

#include "Block.hpp"
#include "../model/BlockModel.hpp"
#include "../model/EntityModel.hpp"
#include "../model/OutlineModel.hpp"
#include "../model/shapes.hpp"

#include <glm/glm.hpp>


namespace blank {

/// single 1x1x1 cube
/// attributes of a type of block
struct BlockType {

	const Shape *shape;
	glm::vec3 color;
	glm::vec3 outline_color;

	// a string to display to the user
	std::string label;

	Block::Type id;

	// light level that blocks of this type emit
	int luminosity;

	// whether to draw
	bool visible;
	// if true, stops light from propagating and fixes level to luminosity
	bool block_light;

	// whether to check for collisions at all
	bool collision;
	// if the block should be impenetrable
	bool collide_block;

	struct Faces {
		bool face[Block::FACE_COUNT];
		Faces &operator =(const Faces &other) noexcept {
			for (int i = 0; i < Block::FACE_COUNT; ++i) {
				face[i] = other.face[i];
			}
			return *this;
		}
		bool operator [](Block::Face f) const noexcept {
			return face[f];
		}
	} fill;

	explicit BlockType(
		bool v = false,
		const glm::vec3 &color = { 1, 1, 1 },
		const Shape *shape = &DEFAULT_SHAPE
	) noexcept;

	static const NullShape DEFAULT_SHAPE;

	bool FaceFilled(const Block &block, Block::Face face) const noexcept {
		return fill[block.OrientedFace(face)];
	}

	void FillEntityModel(
		EntityModel::Buffer &m,
		const glm::mat4 &transform = glm::mat4(1.0f),
		EntityModel::Index idx_offset = 0
	) const noexcept;
	void FillBlockModel(
		BlockModel::Buffer &m,
		const glm::mat4 &transform = glm::mat4(1.0f),
		BlockModel::Index idx_offset = 0
	) const noexcept;
	void FillOutlineModel(
		OutlineModel::Buffer &m,
		const glm::vec3 &pos_offset = { 0, 0, 0 },
		OutlineModel::Index idx_offset = 0
	) const noexcept;

};

}

#endif
