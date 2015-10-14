#include "Model.hpp"
#include "Instance.hpp"
#include "Skeletons.hpp"

#include "Shape.hpp"
#include "ShapeRegistry.hpp"
#include "../app/TextureIndex.hpp"
#include "../graphics/DirectionalLighting.hpp"
#include "../graphics/EntityMesh.hpp"

#include <iostream>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/io.hpp>


namespace blank {

Instance::Instance()
: model(nullptr)
, state()
, mesh()
, tex_map()
, hsl_mod(0.0f, 1.0f, 1.0f)
, rgb_mod(1.0f) {

}

Instance::~Instance() {

}

Instance::Instance(const Instance &other)
: model(other.model)
, state(other.state)
, mesh()
, tex_map(other.tex_map)
, hsl_mod(other.hsl_mod)
, rgb_mod(other.rgb_mod) {

}

Instance &Instance::operator =(const Instance &other) {
	model = other.model;
	state = other.state;
	mesh.clear();
	tex_map = other.tex_map;
	hsl_mod = other.hsl_mod;
	rgb_mod = other.rgb_mod;
	return *this;
}

void Instance::Render(const glm::mat4 &M, DirectionalLighting &prog) {
	if (mesh.empty()) {
		std::cout << "building meshes for instance" << std::endl;
		mesh.resize(state.size());
		model->RootPart().LoadMeshes(*this);
	}
	model->RootPart().Render(M, *this, prog);
}

void Instance::SetTextures(const std::vector<float> &t) {
	tex_map = t;
	mesh.clear();
}

void Instance::SetHSLModifier(const glm::vec3 &m) {
	hsl_mod = m;
	mesh.clear();
}

void Instance::SetRGBModifier(const glm::vec3 &m) {
	rgb_mod = m;
	mesh.clear();
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
	inst.mesh.clear();
	inst.state.resize(part.size());
}


Part::Part()
: id(0)
, bounds{ glm::vec3(0.0f), glm::vec3(0.0f) }
, initial()
, shape(nullptr)
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

glm::mat4 Part::LocalTransform(const Instance &inst) const noexcept {
	glm::mat4 transform(toMat4(initial.orientation * inst.state[id].orientation));
	transform[3] = glm::vec4(initial.position + inst.state[id].position, 1.0f);
	return transform;
}

glm::mat4 Part::GlobalTransform(const Instance &inst) const noexcept {
	if (parent) {
		return parent->GlobalTransform(inst) * LocalTransform(inst);
	} else {
		return LocalTransform(inst);
	}
}

namespace {

EntityMesh::Buffer buf;

}

void Part::LoadMeshes(Instance &inst) const {
	if (shape && shape->IndexCount() > 0) {
		buf.Clear();
		buf.hsl_mods.resize(shape->VertexCount(), inst.hsl_mod);
		buf.rgb_mods.resize(shape->VertexCount(), inst.rgb_mod);
		shape->Fill(buf, inst.tex_map);
		inst.mesh[id].reset(new EntityMesh());
		inst.mesh[id]->Update(buf);
	} else {
		inst.mesh[id].reset();
	}
	for (const Part &part : children) {
		part.LoadMeshes(inst);
	}
}

void Part::Render(
	const glm::mat4 &M,
	const Instance &inst,
	DirectionalLighting &prog
) const {
	glm::mat4 transform = M * LocalTransform(inst);
	if (inst.mesh[id]) {
		prog.SetM(transform);
		inst.mesh[id]->Draw();
	}
	for (const Part &part : children) {
		part.Render(transform, inst, prog);
	}
}


Skeletons::Skeletons()
: skeletons() {

}

Skeletons::~Skeletons() {

}

void Skeletons::Load(const ShapeRegistry &shapes) {
	skeletons.clear();
	skeletons.reserve(4);
	AABB bounds{{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }};
	const Shape *shape = &shapes.Get("player_head_block");
	{
		skeletons.emplace_back(new Model);
		skeletons[0]->ID(1);
		skeletons[0]->RootPart().bounds = bounds;
		skeletons[0]->RootPart().shape = shape;
		skeletons[0]->Enumerate();
	}
	{
		skeletons.emplace_back(new Model);
		skeletons[1]->ID(2);
		skeletons[1]->RootPart().bounds = bounds;
		skeletons[1]->RootPart().shape = shape;
		skeletons[1]->Enumerate();
	}
	{
		skeletons.emplace_back(new Model);
		skeletons[2]->ID(3);
		skeletons[2]->RootPart().bounds = bounds;
		skeletons[2]->RootPart().shape = shape;
		skeletons[2]->Enumerate();
	}
	{
		skeletons.emplace_back(new Model);
		skeletons[3]->ID(4);
		skeletons[3]->RootPart().bounds = bounds;
		skeletons[3]->RootPart().shape = shape;
		skeletons[3]->Enumerate();
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
