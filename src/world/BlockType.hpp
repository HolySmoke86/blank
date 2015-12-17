#ifndef BLANK_WORLD_BLOCKTYPE_HPP_
#define BLANK_WORLD_BLOCKTYPE_HPP_

#include "Block.hpp"
#include "BlockGravity.hpp"
#include "../graphics/BlockMesh.hpp"
#include "../graphics/EntityMesh.hpp"
#include "../graphics/PrimitiveMesh.hpp"
#include "../model/Shape.hpp"

#include <glm/glm.hpp>
#include <vector>


namespace blank {

class ResourceIndex;
class ShapeRegistry;
class TokenStreamReader;

/// single 1x1x1 cube
/// attributes of a type of block
struct BlockType {

	const Shape *shape;
	std::vector<float> textures;
	glm::tvec3<unsigned char> hsl_mod;
	glm::tvec3<unsigned char> rgb_mod;
	glm::tvec3<unsigned char> outline_color;

	/// gravity configuration or null if not emitting gravity
	std::unique_ptr<BlockGravity> gravity;

	/// a string identifying in contexts where numbers just won't do
	/// must be unique within any given set
	std::string name;
	/// a string to display to the user
	std::string label;

	int place_sound;
	int remove_sound;

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

	BlockType() noexcept;

	/// clone values of given type
	/// this copies everything except for ID, name, label, and gravity
	void Copy(const BlockType &) noexcept;

	void Read(
		TokenStreamReader &in,
		ResourceIndex &snd_index,
		ResourceIndex &tex_index,
		const ShapeRegistry &shapes);

	bool FaceFilled(const Block &block, Block::Face face) const noexcept {
		return shape && shape->FaceFilled(block.OrientedFace(face));
	}

	void FillEntityMesh(
		EntityMesh::Buffer &m,
		const glm::mat4 &transform = glm::mat4(1.0f)
	) const noexcept;
	void FillBlockMesh(
		BlockMesh::Buffer &m,
		const glm::mat4 &transform = glm::mat4(1.0f),
		BlockMesh::Index idx_offset = 0
	) const noexcept;
	void OutlinePrimitiveMesh(PrimitiveMesh::Buffer &) const noexcept;

};

}

#endif
