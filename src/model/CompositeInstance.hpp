#ifndef BLANK_MODEL_COMPOSITEINSTANCE_HPP_
#define BLANK_MODEL_COMPOSITEINSTANCE_HPP_

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class CompositeModel;
class DirectionalLighting;

// TODO: this doesn't have to be a tree, actually
//       linearizing might be a good opportunity to optimize
class CompositeInstance {

	friend class CompositeModel;

public:
	CompositeInstance();

	operator bool() const noexcept { return part_model; }
	const CompositeModel &GetModel() const noexcept { return *part_model; }

	const glm::vec3 &Position() const noexcept { return position; }
	void Position(const glm::vec3 &p) noexcept { position = p; }

	const glm::quat &Orientation() const noexcept { return orientation; }
	void Orientation(const glm::quat &o) noexcept { orientation = o; }

	glm::mat4 LocalTransform() const noexcept;
	glm::mat4 GlobalTransform() const noexcept;

	void Render(const glm::mat4 &, DirectionalLighting &) const;

private:
	CompositeInstance &AddPart();
	bool HasParent() const noexcept { return parent; }
	CompositeInstance &Parent() const noexcept { return *parent; }
	bool IsRoot() const noexcept { return !HasParent(); }

private:
	const CompositeModel *part_model;
	CompositeInstance *parent;

	glm::vec3 position;
	glm::quat orientation;

	std::vector<CompositeInstance> parts;

};

}

#endif
