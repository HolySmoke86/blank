#ifndef BLANK_WORLD_BLOCKTYPE_HPP_
#define BLANK_WORLD_BLOCKTYPE_HPP_

#include "Block.hpp"
#include "BlockGravity.hpp"
#include "../graphics/BlockMesh.hpp"
#include "../graphics/EntityMesh.hpp"
#include "../graphics/glm.hpp"
#include "../graphics/PrimitiveMesh.hpp"
#include "../model/Shape.hpp"

#include <limits>
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
	TVEC3<unsigned char, glm::precision(0)> hsl_mod;
	TVEC3<unsigned char, glm::precision(0)> rgb_mod;
	TVEC3<unsigned char, glm::precision(0)> outline_color;

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
	class Distribution {

	public:
		Distribution(float min, float mid, float max)
		: xmin(min), xmid(mid), xmax(max) { Update(); }

		bool Valid(float x) const noexcept {
			return x >= xmin && x <= xmax;
		}
		float Map(float x) const noexcept {
			// previous algo as was used by Generator
			//return 4.0f - ((x - xmid) * (x - xmid));

			// linear mapping of [min,mid,max] to [-1,0,1]
			x -= xmid;
			x *= (x < 0) ? inv_neg : inv_pos;

			// smoothing: x^4 - 2x^2 + 1
			x *= x;
			return x * x - 2.0f * x + 1.0f;
		}

		void Min(float m) noexcept { xmin = m; Update(); }
		float Min() const noexcept { return xmin; }
		void Mid(float m) noexcept { xmid = m; Update(); }
		float Mid() const noexcept { return xmid; }
		void Max(float m) noexcept { xmax = m; Update(); }
		float Max() const noexcept { return xmax; }

	private:
		void Update() {
			float abs_min = std::abs(xmin - xmid);
			inv_neg = abs_min < std::numeric_limits<float>::epsilon() ? 0.0f : 1.0f / abs_min;
			float abs_max = std::abs(xmax - xmid);
			inv_pos = abs_max < std::numeric_limits<float>::epsilon() ? 0.0f : 1.0f / abs_max;
		}

		float xmin;
		float xmid;
		float xmax;
		float inv_neg;
		float inv_pos;

	};

	Distribution solidity;
	Distribution humidity;
	Distribution temperature;
	Distribution richness;
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
