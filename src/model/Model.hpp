#ifndef BLANK_MODEL_MODEL_HPP_
#define BLANK_MODEL_MODEL_HPP_

#include "Part.hpp"

#include <cstdint>
#include <list>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class Instance;
class EntityMesh;

class Model {

public:
	Model();

	Model(const Model &) = delete;
	Model &operator =(const Model &) = delete;

	std::uint32_t ID() const noexcept { return id; }
	void ID(std::uint32_t i) noexcept { id = i; }

	Part &RootPart() noexcept { return root; }
	const Part &RootPart() const noexcept { return root; }
	Part &GetPart(std::size_t i) noexcept { return *part[i]; }
	const Part &GetPart(std::size_t i) const noexcept { return *part[i]; }

	void Enumerate();
	void Instantiate(Instance &) const;

private:
	std::uint32_t id;
	Part root;
	std::vector<Part *> part;

};

}

#endif
