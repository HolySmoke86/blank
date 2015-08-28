#include "CompositeModel.hpp"
#include "CompositeInstance.hpp"

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


void CompositeModel::Instantiate(CompositeInstance &inst) const {
	inst.part_model = this;
	inst.position = position;
	inst.orientation = orientation;
	inst.parts.clear();
	inst.parts.reserve(parts.size());
	for (const CompositeModel &part : parts) {
		part.Instantiate(inst.AddPart());
	}
}


CompositeInstance::CompositeInstance()
: part_model(nullptr)
, parent(nullptr)
, position(0.0f)
, orientation(1.0f, 0.0f, 0.0f, 0.0f)
, parts() {

}


CompositeInstance &CompositeInstance::AddPart() {
	parts.emplace_back();
	parts.back().parent = this;
	return parts.back();
}


glm::mat4 CompositeInstance::LocalTransform() const noexcept {
	glm::mat4 transform(toMat4(orientation));
	transform[3].x = position.x;
	transform[3].y = position.y;
	transform[3].z = position.z;
	return transform;
}

glm::mat4 CompositeInstance::GlobalTransform() const noexcept {
	if (HasParent()) {
		return Parent().GlobalTransform() * LocalTransform();
	} else {
		return LocalTransform();
	}
}


void CompositeInstance::Render(const glm::mat4 &M, DirectionalLighting &prog) const {
	glm::mat4 transform(M * LocalTransform());
	if (part_model->HasNodeModel()) {
		prog.SetM(transform);
		part_model->NodeModel().Draw();
	}
	for (const CompositeInstance &part : parts) {
		part.Render(transform, prog);
	}
}

}
