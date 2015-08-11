#include "CompositeModel.hpp"

#include "EntityModel.hpp"
#include "../graphics/DirectionalLighting.hpp"

#include <glm/gtx/quaternion.hpp>


namespace blank {

CompositeModel::CompositeModel()
: node_model(nullptr)
, position(0.0f)
, orientation(1.0f, 0.0f, 0.0f, 0.0f)
, parts() {

}


CompositeModel &CompositeModel::AddPart() {
	parts.emplace_back();
	parts.back().parent = this;
	return parts.back();
}


glm::mat4 CompositeModel::LocalTransform() const noexcept {
	glm::mat4 transform(toMat4(orientation));
	transform[3].x = position.x;
	transform[3].y = position.y;
	transform[3].z = position.z;
	return transform;
}

glm::mat4 CompositeModel::GlobalTransform() const noexcept {
	if (HasParent()) {
		return Parent().GlobalTransform() * LocalTransform();
	} else {
		return LocalTransform();
	}
}


void CompositeModel::Render(const glm::mat4 &M, DirectionalLighting &prog) const {
	glm::mat4 transform(M * LocalTransform());
	if (HasNodeModel()) {
		prog.SetM(transform);
		NodeModel().Draw();
	}
	for (const CompositeModel &part : parts) {
		part.Render(transform, prog);
	}
}

}
