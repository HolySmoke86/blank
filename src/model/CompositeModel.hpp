#ifndef BLANK_MODEL_COMPOSITEMODEL_HPP_
#define BLANK_MODEL_COMPOSITEMODEL_HPP_

#include <list>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class DirectionalLighting;
class EntityModel;

class CompositeModel {

public:
	CompositeModel();

	CompositeModel(const CompositeModel &) = delete;
	CompositeModel &operator =(const CompositeModel &) = delete;

	const glm::vec3 &Position() const noexcept { return position; }
	void Position(const glm::vec3 &p) noexcept { position = p; }

	const glm::quat &Orientation() const noexcept { return orientation; }
	void Orientation(const glm::quat &o) noexcept { orientation = o; }

	bool HasNodeModel() const noexcept { return node_model; }
	void SetNodeModel(const EntityModel *m) noexcept { node_model = m; }

	const EntityModel &NodeModel() const noexcept { return *node_model; }

	CompositeModel &AddPart();
	bool HasParent() const noexcept { return parent; }
	CompositeModel &Parent() const noexcept { return *parent; }
	bool IsRoot() const noexcept { return !HasParent(); }

	glm::mat4 LocalTransform() const noexcept;
	glm::mat4 GlobalTransform() const noexcept;

	void Render(const glm::mat4 &, DirectionalLighting &) const;

private:
	CompositeModel *parent;
	const EntityModel *node_model;

	glm::vec3 position;
	glm::quat orientation;

	std::list<CompositeModel> parts;

};

}

#endif
