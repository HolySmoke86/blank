#include "Model.hpp"
#include "Instance.hpp"
#include "Skeletons.hpp"

#include "Shape.hpp"
#include "ShapeRegistry.hpp"
#include "../app/TextureIndex.hpp"
#include "../graphics/DirectionalLighting.hpp"
#include "../graphics/EntityMesh.hpp"

#include <glm/gtx/quaternion.hpp>


namespace blank {

Instance::Instance()
: model(nullptr)
, state() {

}

void Instance::Render(const glm::mat4 &M, DirectionalLighting &prog) const {
	model->RootPart().Render(M, state, prog);
}


Model::Model()
: id(0)
, root()
, part() {

}

void Model::Enumerate() {
	part.clear();
	part.resize(root.Enumerate(0), nullptr);
	root.Index(part);
}

void Model::Instantiate(Instance &inst) const {
	inst.model = this;
	inst.state.clear();
	inst.state.resize(part.size());
}


Part::Part()
: id(0)
, bounds{ glm::vec3(0.0f), glm::vec3(0.0f) }
, initial()
, mesh(nullptr)
, parent(nullptr)
, children() {

}

Part::~Part() {

}

Part &Part::AddChild() {
	children.emplace_back();
	children.back().parent = this;
	return children.back();
}

std::uint16_t Part::Enumerate(std::uint16_t counter) noexcept {
	id = counter++;
	for (Part &part : children) {
		counter = part.Enumerate(counter);
	}
	return counter;
}

void Part::Index(std::vector<Part *> &index) noexcept {
	index[id] = this;
	for (Part &part : children) {
		part.Index(index);
	}
}

glm::mat4 Part::LocalTransform(
	const std::vector<State> &state
) const noexcept {
	glm::mat4 transform(toMat4(initial.orientation * state[id].orientation));
	transform[3] = glm::vec4(initial.position + state[id].position, 1.0f);
	return transform;
}

glm::mat4 Part::GlobalTransform(
	const std::vector<State> &state
) const noexcept {
	if (parent) {
		return parent->GlobalTransform(state) * LocalTransform(state);
	} else {
		return LocalTransform(state);
	}
}

void Part::Render(
	const glm::mat4 &M,
	const std::vector<State> &state,
	DirectionalLighting &prog
) const {
	glm::mat4 transform = M * LocalTransform(state);
	if (mesh) {
		prog.SetM(transform);
		mesh->Draw();
	}
	for (const Part &part : children) {
		part.Render(transform, state, prog);
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
	AABB bounds{{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }};
	{
		skeletons.emplace_back(new Model);
		skeletons[0]->ID(1);
		skeletons[0]->RootPart().bounds = bounds;
		skeletons[0]->Enumerate();
	}
	{
		skeletons.emplace_back(new Model);
		skeletons[1]->ID(2);
		skeletons[1]->RootPart().bounds = bounds;
		skeletons[1]->Enumerate();
	}
	{
		skeletons.emplace_back(new Model);
		skeletons[2]->ID(3);
		skeletons[2]->RootPart().bounds = bounds;
		skeletons[2]->Enumerate();
	}
	{
		skeletons.emplace_back(new Model);
		skeletons[3]->ID(4);
		skeletons[3]->RootPart().bounds = bounds;
		skeletons[3]->Enumerate();
	}
}

void Skeletons::Load(const ShapeRegistry &shapes, TextureIndex &tex_index) {
	LoadHeadless();
	meshes.resize(4);
	const Shape &shape = shapes.Get("player_head_block");
	EntityMesh::Buffer buf;
	std::vector<float> tex_map;
	tex_map.push_back(tex_index.GetID("rock-1"));
	tex_map.push_back(tex_index.GetID("rock-face"));
	buf.Reserve(shape.VertexCount(), shape.IndexCount());
	{
		shape.Fill(buf, tex_map);
		buf.hsl_mods.resize(shape.VertexCount(), { 0.0f, 1.0f, 1.0f });
		buf.rgb_mods.resize(shape.VertexCount(), { 1.0f, 1.0f, 0.0f });
		meshes[0].Update(buf);
		skeletons[0]->RootPart().mesh = &meshes[0];
	}
	{
		buf.Clear();
		shape.Fill(buf, tex_map);
		buf.hsl_mods.resize(shape.VertexCount(), { 0.0f, 1.0f, 1.0f });
		buf.rgb_mods.resize(shape.VertexCount(), { 0.0f, 1.0f, 1.0f });
		meshes[1].Update(buf);
		skeletons[1]->RootPart().mesh = &meshes[1];
	}
	{
		buf.Clear();
		shape.Fill(buf, tex_map);
		buf.hsl_mods.resize(shape.VertexCount(), { 0.0f, 1.0f, 1.0f });
		buf.rgb_mods.resize(shape.VertexCount(), { 1.0f, 0.0f, 1.0f });
		meshes[2].Update(buf);
		skeletons[2]->RootPart().mesh = &meshes[2];
	}
	{
		buf.Clear();
		shape.Fill(buf, tex_map);
		buf.hsl_mods.resize(shape.VertexCount(), { 0.0f, 1.0f, 1.0f });
		buf.rgb_mods.resize(shape.VertexCount(), { 1.0f, 0.25f, 0.5f });
		meshes[3].Update(buf);
		skeletons[3]->RootPart().mesh = &meshes[3];
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
