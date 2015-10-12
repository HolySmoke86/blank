#include "Model.hpp"
#include "Instance.hpp"
#include "Skeletons.hpp"

#include "shapes.hpp"
#include "../graphics/DirectionalLighting.hpp"
#include "../graphics/EntityMesh.hpp"

#include <glm/gtx/quaternion.hpp>


namespace blank {

Model::Model()
: parent(nullptr)
, node_mesh(nullptr)
, id(0)
, bounds{{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }}
, position(0.0f)
, orientation(1.0f, 0.0f, 0.0f, 0.0f)
, parts() {

}


Model &Model::AddPart() {
	parts.emplace_back();
	parts.back().parent = this;
	return parts.back();
}


glm::mat4 Model::LocalTransform() const noexcept {
	glm::mat4 transform(toMat4(orientation));
	transform[3].x = position.x;
	transform[3].y = position.y;
	transform[3].z = position.z;
	return transform;
}

glm::mat4 Model::GlobalTransform() const noexcept {
	if (HasParent()) {
		return Parent().GlobalTransform() * LocalTransform();
	} else {
		return LocalTransform();
	}
}


void Model::Instantiate(Instance &inst) const {
	inst.part_model = this;
	inst.position = position;
	inst.orientation = orientation;
	inst.parts.clear();
	inst.parts.reserve(parts.size());
	for (const Model &part : parts) {
		part.Instantiate(inst.AddPart());
	}
}


Instance::Instance()
: part_model(nullptr)
, parent(nullptr)
, position(0.0f)
, orientation(1.0f, 0.0f, 0.0f, 0.0f)
, parts() {

}


Instance &Instance::AddPart() {
	parts.emplace_back();
	parts.back().parent = this;
	return parts.back();
}


glm::mat4 Instance::LocalTransform() const noexcept {
	glm::mat4 transform(toMat4(orientation));
	transform[3].x = position.x;
	transform[3].y = position.y;
	transform[3].z = position.z;
	return transform;
}

glm::mat4 Instance::GlobalTransform() const noexcept {
	if (HasParent()) {
		return Parent().GlobalTransform() * LocalTransform();
	} else {
		return LocalTransform();
	}
}


void Instance::Render(const glm::mat4 &M, DirectionalLighting &prog) const {
	glm::mat4 transform(M * LocalTransform());
	if (part_model->HasNodeMesh()) {
		prog.SetM(transform);
		part_model->NodeMesh().Draw();
	}
	for (const Instance &part : parts) {
		part.Render(transform, prog);
	}
}


Skeletons::Skeletons()
: skeletons()
, meshes() {

}

Skeletons::~Skeletons() {

}

void Skeletons::LoadHeadless() {
	skeletons.clear();
	skeletons.reserve(4);
	{
		AABB bounds{{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }};
		skeletons.emplace_back(new Model);
		skeletons[0]->ID(1);
		skeletons[0]->Bounds(bounds);
	}
	{
		AABB bounds{{ -0.5f, -0.25f, -0.5f }, { 0.5f, 0.25f, 0.5f }};
		skeletons.emplace_back(new Model);
		skeletons[1]->ID(2);
		skeletons[1]->Bounds(bounds);
	}
	{
		AABB bounds{{ -0.25f, -0.5f, -0.25f }, { 0.25f, 0.5f, 0.25f }};
		skeletons.emplace_back(new Model);
		skeletons[2]->ID(3);
		skeletons[2]->Bounds(bounds);
	}
	{
		AABB bounds{{ -0.25f, -0.5f, -0.35f }, { 0.25f, 0.5f, 0.35f }};
		skeletons.emplace_back(new Model);
		skeletons[3]->ID(4);
		skeletons[3]->Bounds(bounds);
	}
}

void Skeletons::Load() {
	LoadHeadless();
	meshes.resize(4);
	EntityMesh::Buffer buf;
	{
		CuboidShape shape(skeletons[0]->Bounds());
		shape.Vertices(buf, 3.0f);
		buf.hsl_mods.resize(shape.VertexCount(), { 0.0f, 1.0f, 1.0f });
		buf.rgb_mods.resize(shape.VertexCount(), { 1.0f, 1.0f, 0.0f });
		meshes[0].Update(buf);
		skeletons[0]->SetNodeMesh(&meshes[0]);
	}
	{
		CuboidShape shape(skeletons[1]->Bounds());
		buf.Clear();
		shape.Vertices(buf, 0.0f);
		buf.hsl_mods.resize(shape.VertexCount(), { 0.0f, 1.0f, 1.0f });
		buf.rgb_mods.resize(shape.VertexCount(), { 0.0f, 1.0f, 1.0f });
		meshes[1].Update(buf);
		skeletons[1]->SetNodeMesh(&meshes[1]);
	}
	{
		StairShape shape(skeletons[2]->Bounds(), { 0.4f, 0.4f });
		buf.Clear();
		shape.Vertices(buf, 1.0f);
		buf.hsl_mods.resize(shape.VertexCount(), { 0.0f, 1.0f, 1.0f });
		buf.rgb_mods.resize(shape.VertexCount(), { 1.0f, 0.0f, 1.0f });
		meshes[2].Update(buf);
		skeletons[2]->SetNodeMesh(&meshes[2]);
	}
	{
		CuboidShape shape(skeletons[3]->Bounds());
		buf.Clear();
		shape.Vertices(buf, 2.0f);
		buf.hsl_mods.resize(shape.VertexCount(), { 0.0f, 1.0f, 1.0f });
		buf.rgb_mods.resize(shape.VertexCount(), { 1.0f, 0.25f, 0.5f });
		meshes[3].Update(buf);
		skeletons[3]->SetNodeMesh(&meshes[3]);
	}
}

Model *Skeletons::ByID(std::uint16_t id) noexcept {
	if (id == 0 || id > skeletons.size()) {
		return nullptr;
	} else {
		return skeletons[id - 1].get();
	}
}

const Model *Skeletons::ByID(std::uint16_t id) const noexcept {
	if (id == 0 || id > skeletons.size()) {
		return nullptr;
	} else {
		return skeletons[id - 1].get();
	}
}

}
