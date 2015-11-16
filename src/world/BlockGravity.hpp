#ifndef BLANK_WORLD_BLOCKGRAVITY_HPP_
#define BLANK_WORLD_BLOCKGRAVITY_HPP_

#include <memory>
#include <glm/glm.hpp>


namespace blank {

class TokenStreamReader;

struct BlockGravity {

	virtual ~BlockGravity();

	/// get gravitational force for a unit mass at relative position diff
	/// diff is target - block, i.e. pointing from block towards the target
	/// orientation of the block in question is given by M
	/// return value should be world absolute
	virtual glm::vec3 GetGravity(const glm::vec3 &diff, const glm::mat4 &M) const noexcept = 0;

	static std::unique_ptr<BlockGravity> Read(TokenStreamReader &in);

};

}

#endif
