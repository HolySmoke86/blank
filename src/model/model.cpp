#include "Model.hpp"
#include "ModelRegistry.hpp"
#include "Instance.hpp"
#include "Part.hpp"

#include "Shape.hpp"
#include "ShapeRegistry.hpp"
#include "../io/TokenStreamReader.hpp"
#include "../graphics/DirectionalLighting.hpp"
#include "../graphics/EntityMesh.hpp"
#include "../shared/ResourceIndex.hpp"

#include <iostream>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/io.hpp>


namespace blank {

Instance::Instance()
: model(nullptr)
, state() {

}

Instance::~Instance() {

}

Part::State &Instance::BodyState() noexcept {
	return state[model->GetBodyPart().ID()];
}

glm::mat4 Instance::BodyTransform() const noexcept {
	return model->GetBodyPart().GlobalTransform(*this);
}

Part::State &Instance::EyesState() noexcept {
	return state[model->GetEyesPart().ID()];
}

glm::mat4 Instance::EyesTransform() const noexcept {
	return model->GetEyesPart().GlobalTransform(*this);
}

void Instance::Render(const glm::mat4 &M, DirectionalLighting &prog) {
	model->RootPart().Render(M, *this, prog);
}


Model::Model()
: id(0)
, root()
, part()
, body_id(0)
, eyes_id(0) {

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


ModelRegistry::ModelRegistry()
: models()
, name_index() {

}

Model &ModelRegistry::Add(const std::string &name) {
	models.emplace_back(new Model());
	models.back()->ID(models.size());
	name_index[name] = &*models.back();
	return *models.back();
}

Model &ModelRegistry::Get(const std::string &name) {
	auto entry = name_index.find(name);
	if (entry != name_index.end()) {
		return *entry->second;
	} else {
		throw std::runtime_error("unknown model " + name);
	}
}

const Model &ModelRegistry::Get(const std::string &name) const {
	auto entry = name_index.find(name);
	if (entry != name_index.end()) {
		return *entry->second;
	} else {
		throw std::runtime_error("unknown model " + name);
	}
}


Part::Part()
: parent(nullptr)
, shape(nullptr)
, children()
, tex_map()
, mesh()
, initial()
, hsl_mod(0.0f, 1.0f, 1.0f)
, rgb_mod(1.0f, 1.0f, 1.0f)
, id(0) {

}

Part::~Part() {

}

void Part::Read(TokenStreamReader &in, ResourceIndex &tex_index, const ShapeRegistry &shapes) {
	std::string name;
	std::string shape_name;
	std::string tex_name;
	in.Skip(Token::ANGLE_BRACKET_OPEN);
	while (in.HasMore() && in.Peek().type != Token::ANGLE_BRACKET_CLOSE) {
		in.ReadIdentifier(name);
		in.Skip(Token::EQUALS);
		if (name == "shape") {
			in.ReadIdentifier(shape_name);
			shape = &shapes.Get(shape_name);
		} else if (name == "position") {
			in.ReadVec(initial.position);
		} else if (name == "orientation") {
			in.ReadQuat(initial.orientation);
		} else if (name == "hsl_mod") {
			in.ReadVec(hsl_mod);
		} else if (name == "rgb_mod") {
			in.ReadVec(rgb_mod);
		} else if (name == "textures") {
			in.Skip(Token::BRACKET_OPEN);
			while (in.HasMore() && in.Peek().type != Token::BRACKET_CLOSE) {
				in.ReadString(tex_name);
				tex_map.push_back(tex_index.GetID(tex_name));
				if (in.Peek().type == Token::COMMA) {
					in.Skip(Token::COMMA);
				}
			}
			in.Skip(Token::BRACKET_CLOSE);
		} else if (name == "children") {
			in.Skip(Token::BRACKET_OPEN);
			while (in.HasMore() && in.Peek().type != Token::BRACKET_CLOSE) {
				Part &child = AddChild();
				child.Read(in, tex_index, shapes);
				if (in.Peek().type == Token::COMMA) {
					in.Skip(Token::COMMA);
				}
			}
			in.Skip(Token::BRACKET_CLOSE);
		} else {
			while (in.HasMore() && in.Peek().type != Token::SEMICOLON) {
				in.Next();
			}
		}
		in.Skip(Token::SEMICOLON);
	}
	in.Skip(Token::ANGLE_BRACKET_CLOSE);
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

void Part::Render(
	const glm::mat4 &M,
	const Instance &inst,
	DirectionalLighting &prog
) const {
	glm::mat4 transform = M * LocalTransform(inst);
	if (shape && shape->IndexCount() > 0) {
		if (!mesh) {
			buf.Clear();
			buf.hsl_mods.resize(shape->VertexCount(), hsl_mod);
			buf.rgb_mods.resize(shape->VertexCount(), rgb_mod);
			shape->Fill(buf, tex_map);
			mesh.reset(new EntityMesh());
			mesh->Update(buf);
		}
		prog.SetM(transform);
		mesh->Draw();
	}
	for (const Part &part : children) {
		part.Render(transform, inst, prog);
	}
}

}
