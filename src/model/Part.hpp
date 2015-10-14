#ifndef BLAMK_MODEL_PART_HPP_
#define BLAMK_MODEL_PART_HPP_

#include "geometry.hpp"

#include <cstdint>
#include <list>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class DirectionalLighting;
class Instance;
class Model;
class Shape;

struct Part {

	std::uint16_t id;
	AABB bounds;
	struct State {
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	} initial;
	const Shape *shape;

	Part();
	~Part();

	Part &AddChild();
	const std::list<Part> &Children() const noexcept { return children; }

	std::uint16_t Enumerate(std::uint16_t) noexcept;
	void Index(std::vector<Part *> &) noexcept;

	glm::mat4 LocalTransform(const Instance &) const noexcept;
	glm::mat4 GlobalTransform(const Instance &) const noexcept;

	void LoadMeshes(Instance &) const;
	void Render(
		const glm::mat4 &,
		const Instance &,
		DirectionalLighting &) const;

private:
	const Part *parent;
	std::list<Part> children;

};

}

#endif
