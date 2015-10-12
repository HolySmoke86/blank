#ifndef BLANK_MODEL_COMPOSITEMODEL_HPP_
#define BLANK_MODEL_COMPOSITEMODEL_HPP_

#include "geometry.hpp"

#include <cstdint>
#include <list>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class CompositeInstance;
class EntityMesh;

class CompositeModel {

public:
	CompositeModel();

	CompositeModel(const CompositeModel &) = delete;
	CompositeModel &operator =(const CompositeModel &) = delete;

	std::uint32_t ID() const noexcept { return id; }
	void ID(std::uint32_t i) noexcept { id = i; }

	const AABB &Bounds() const noexcept { return bounds; }
	void Bounds(const AABB &b) noexcept { bounds = b; }

	const glm::vec3 &Position() const noexcept { return position; }
	void Position(const glm::vec3 &p) noexcept { position = p; }

	const glm::quat &Orientation() const noexcept { return orientation; }
	void Orientation(const glm::quat &o) noexcept { orientation = o; }

	bool HasNodeMesh() const noexcept { return node_mesh; }
	void SetNodeMesh(const EntityMesh *m) noexcept { node_mesh = m; }

	const EntityMesh &NodeMesh() const noexcept { return *node_mesh; }

	CompositeModel &AddPart();
	bool HasParent() const noexcept { return parent; }
	CompositeModel &Parent() const noexcept { return *parent; }
	bool IsRoot() const noexcept { return !HasParent(); }

	glm::mat4 LocalTransform() const noexcept;
	glm::mat4 GlobalTransform() const noexcept;

	void Instantiate(CompositeInstance &) const;

private:
	CompositeModel *parent;
	const EntityMesh *node_mesh;

	std::uint32_t id;

	AABB bounds;

	glm::vec3 position;
	glm::quat orientation;

	std::list<CompositeModel> parts;

};

}

#endif
