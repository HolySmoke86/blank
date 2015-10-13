#ifndef BLANK_WORLD_BLOCKTYPE_HPP_
#define BLANK_WORLD_BLOCKTYPE_HPP_

#include "Block.hpp"
#include "../graphics/BlockMesh.hpp"
#include "../graphics/EntityMesh.hpp"
#include "../graphics/OutlineMesh.hpp"
#include "../model/bounds.hpp"

#include <glm/glm.hpp>
#include <vector>


namespace blank {

/// single 1x1x1 cube
/// attributes of a type of block
struct BlockType {

	const CollisionBounds *shape;
	std::vector<float> textures;
	glm::vec3 hsl_mod;
	glm::vec3 rgb_mod;
	glm::vec3 outline_color;

	/// a string to display to the user
	std::string label;

	Block::Type id;

	/// light level that blocks of this type emit
	int luminosity;

	/// whether to draw
	bool visible;
	/// if true, stops light from propagating and fixes level to luminosity
	bool block_light;

	/// whether to check for collisions at all
	bool collision;
	/// if the block should be impenetrable
	bool collide_block;

	// generation properties
	/// whether to use this block in generation at all
	bool generate;
	// min/mid/max points for the respective properties
	// should all be in the (-1,1) range
	float min_solidity;
	float mid_solidity;
	float max_solidity;
	float min_humidity;
	float mid_humidity;
	float max_humidity;
	float min_temperature;
	float mid_temperature;
	float max_temperature;
	float min_richness;
	float mid_richness;
	float max_richness;
	/// commonness factor, random chance is multiplied by this
	float commonness;

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

	BlockType() noexcept;

	static const NullBounds DEFAULT_SHAPE;

	bool FaceFilled(const Block &block, Block::Face face) const noexcept {
		return fill[block.OrientedFace(face)];
	}

	void FillEntityMesh(
		EntityMesh::Buffer &m,
		const glm::mat4 &transform = glm::mat4(1.0f),
		EntityMesh::Index idx_offset = 0
	) const noexcept;
	void FillBlockMesh(
		BlockMesh::Buffer &m,
		const glm::mat4 &transform = glm::mat4(1.0f),
		BlockMesh::Index idx_offset = 0
	) const noexcept;
	void FillOutlineMesh(OutlineMesh::Buffer &m) const noexcept;

};

}

#endif
