#include "CompositeModel.hpp"
#include "CompositeInstance.hpp"
#include "Skeletons.hpp"

#include "EntityModel.hpp"
#include "shapes.hpp"
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


Skeletons::Skeletons()
: skeletons()
, models() {

}

Skeletons::~Skeletons() {

}

void Skeletons::LoadHeadless() {
	skeletons.clear();
	skeletons.reserve(3);
	{
		AABB bounds{{ -0.25f, -0.5f, -0.25f }, { 0.25f, 0.5f, 0.25f }};
		skeletons.emplace_back(new CompositeModel);
		skeletons[0]->Bounds(bounds);
	}
	{
		AABB bounds{{ -0.5f, -0.25f, -0.5f }, { 0.5f, 0.25f, 0.5f }};
		skeletons.emplace_back(new CompositeModel);
		skeletons[1]->Bounds(bounds);
	}
	{
		AABB bounds{{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }};
		skeletons.emplace_back(new CompositeModel);
		skeletons[2]->Bounds(bounds);
	}
}

void Skeletons::Load() {
	LoadHeadless();
	models.resize(3);
	EntityModel::Buffer buf;
	{
		CuboidShape shape(skeletons[0]->Bounds());
		shape.Vertices(buf, 1.0f);
		buf.colors.resize(shape.VertexCount(), { 1.0f, 1.0f, 0.0f });
		models[0].Update(buf);
		skeletons[0]->SetNodeModel(&models[0]);
	}
	{
		CuboidShape shape(skeletons[1]->Bounds());
		buf.Clear();
		shape.Vertices(buf, 2.0f);
		buf.colors.resize(shape.VertexCount(), { 0.0f, 1.0f, 1.0f });
		models[1].Update(buf);
		skeletons[1]->SetNodeModel(&models[1]);
	}
	{
		StairShape shape(skeletons[2]->Bounds(), { 0.4f, 0.4f });
		buf.Clear();
		shape.Vertices(buf, 3.0f);
		buf.colors.resize(shape.VertexCount(), { 1.0f, 0.0f, 1.0f });
		models[2].Update(buf);
		skeletons[2]->SetNodeModel(&models[2]);
	}
}

}
