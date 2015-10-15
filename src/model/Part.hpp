#ifndef BLAMK_MODEL_PART_HPP_
#define BLAMK_MODEL_PART_HPP_

#include "geometry.hpp"

#include <cstdint>
#include <list>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class DirectionalLighting;
class EntityMesh;
class Instance;
class Model;
class Shape;
class ShapeRegistry;
class TextureIndex;
class TokenStreamReader;

struct Part {

public:
	struct State {
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	};

	Part();
	~Part();

	void Read(TokenStreamReader &, TextureIndex &, const ShapeRegistry &);

	Part &AddChild();
	const std::list<Part> &Children() const noexcept { return children; }

	std::uint16_t Enumerate(std::uint16_t) noexcept;
	void Index(std::vector<Part *> &) noexcept;

	glm::mat4 LocalTransform(const Instance &) const noexcept;
	glm::mat4 GlobalTransform(const Instance &) const noexcept;

	void Render(
		const glm::mat4 &,
		const Instance &,
		DirectionalLighting &) const;

private:
	const Part *parent;
	const Shape *shape;
	std::list<Part> children;
	std::vector<float> tex_map;
	mutable std::unique_ptr<EntityMesh> mesh;
	State initial;
	glm::vec3 hsl_mod;
	glm::vec3 rgb_mod;
	std::uint16_t id;

};

}

#endif
