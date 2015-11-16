#include "Block.hpp"
#include "BlockType.hpp"
#include "BlockTypeRegistry.hpp"

#include "../io/TokenStreamReader.hpp"
#include "../model/ShapeRegistry.hpp"
#include "../shared/ResourceIndex.hpp"

#include <iostream>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>


namespace blank {

std::ostream &operator <<(std::ostream &out, const Block &block) {
	return out << "Block(" << block.type << ", " << block.GetFace() << ", " << block.GetTurn() << ')';
}

std::ostream &operator <<(std::ostream &out, const Block::Face &face) {
	switch (face) {
		case Block::FACE_UP:
			out << "FACE_UP";
			break;
		case Block::FACE_DOWN:
			out << "FACE_DOWN";
			break;
		case Block::FACE_RIGHT:
			out << "FACE_RIGHT";
			break;
		case Block::FACE_LEFT:
			out << "FACE_LEFT";
			break;
		case Block::FACE_FRONT:
			out << "FACE_FRONT";
			break;
		case Block::FACE_BACK:
			out << "FACE_BACK";
			break;
		default:
		case Block::FACE_COUNT:
			out << "invalid Block::Face";
			break;
	}
	return out;
}

std::ostream &operator <<(std::ostream &out, const Block::Turn &turn) {
	switch (turn) {
		case Block::TURN_NONE:
			out << "TURN_NONE";
			break;
		case Block::TURN_LEFT:
			out << "TURN_LEFT";
			break;
		case Block::TURN_AROUND:
			out << "TURN_AROUND";
			break;
		case Block::TURN_RIGHT:
			out << "TURN_RIGHT";
			break;
		default:
		case Block::TURN_COUNT:
			out << "invalid Block::Turn";
			break;
	}
	return out;
}


BlockType::BlockType() noexcept
: shape(nullptr)
, textures()
, hsl_mod(0.0f, 1.0f, 1.0f)
, rgb_mod(1.0f, 1.0f, 1.0f)
, outline_color(-1, -1, -1)
, label("some block")
, place_sound(-1)
, remove_sound(-1)
, id(0)
, luminosity(0)
, visible(true)
, block_light(true)
, collision(true)
, collide_block(true)
, generate(false)
, min_solidity(0.5f)
, mid_solidity(0.75f)
, max_solidity(1.0f)
, min_humidity(-1.0f)
, mid_humidity(0.0f)
, max_humidity(1.0f)
, min_temperature(-1.0f)
, mid_temperature(0.0f)
, max_temperature(1.0f)
, min_richness(-1.0f)
, mid_richness(0.0f)
, max_richness(1.0f)
, commonness(1.0f) {

}

void BlockType::Read(
	TokenStreamReader &in,
	ResourceIndex &snd_index,
	ResourceIndex &tex_index,
	const ShapeRegistry &shapes
) {
	std::string name;
	in.Skip(Token::ANGLE_BRACKET_OPEN);
	while (in.Peek().type != Token::ANGLE_BRACKET_CLOSE) {
		in.ReadIdentifier(name);
		in.Skip(Token::EQUALS);
		if (name == "visible") {
			visible = in.GetBool();
		} else if (name == "texture") {
			in.ReadString(name);
			textures.push_back(tex_index.GetID(name));
		} else if (name == "textures") {
			in.Skip(Token::BRACKET_OPEN);
			while (in.Peek().type != Token::BRACKET_CLOSE) {
				in.ReadString(name);
				textures.push_back(tex_index.GetID(name));
				if (in.Peek().type == Token::COMMA) {
					in.Skip(Token::COMMA);
				}
			}
			in.Skip(Token::BRACKET_CLOSE);
		} else if (name == "rgb_mod") {
			in.ReadVec(rgb_mod);
		} else if (name == "hsl_mod") {
			in.ReadVec(hsl_mod);
		} else if (name == "outline") {
			in.ReadVec(outline_color);
		} else if (name == "label") {
			in.ReadString(label);
		} else if (name == "place_sound") {
			in.ReadString(name);
			place_sound = snd_index.GetID(name);
		} else if (name == "remove_sound") {
			in.ReadString(name);
			remove_sound = snd_index.GetID(name);
		} else if (name == "luminosity") {
			luminosity = in.GetInt();
		} else if (name == "block_light") {
			block_light = in.GetBool();
		} else if (name == "collision") {
			collision = in.GetBool();
		} else if (name == "collide_block") {
			collide_block = in.GetBool();
		} else if (name == "generate") {
			generate = in.GetBool();
		} else if (name == "min_solidity") {
			min_solidity = in.GetFloat();
		} else if (name == "mid_solidity") {
			mid_solidity = in.GetFloat();
		} else if (name == "max_solidity") {
			max_solidity = in.GetFloat();
		} else if (name == "min_humidity") {
			min_humidity = in.GetFloat();
		} else if (name == "mid_humidity") {
			mid_humidity = in.GetFloat();
		} else if (name == "max_humidity") {
			max_humidity = in.GetFloat();
		} else if (name == "min_temperature") {
			min_temperature = in.GetFloat();
		} else if (name == "mid_temperature") {
			mid_temperature = in.GetFloat();
		} else if (name == "max_temperature") {
			max_temperature = in.GetFloat();
		} else if (name == "min_richness") {
			min_richness = in.GetFloat();
		} else if (name == "mid_richness") {
			mid_richness = in.GetFloat();
		} else if (name == "max_richness") {
			max_richness = in.GetFloat();
		} else if (name == "commonness") {
			commonness = in.GetFloat();
		} else if (name == "shape") {
			in.ReadIdentifier(name);
			shape = &shapes.Get(name);
		} else {
			std::cerr << "warning: unknown block type property " << name << std::endl;
			while (in.Peek().type != Token::SEMICOLON) {
				in.Next();
			}
		}
		in.Skip(Token::SEMICOLON);
	}
	in.Skip(Token::ANGLE_BRACKET_CLOSE);
}

void BlockType::FillEntityMesh(
	EntityMesh::Buffer &buf,
	const glm::mat4 &transform
) const noexcept {
	if (!shape) return;
	shape->Fill(buf, transform, textures);
	buf.hsl_mods.insert(buf.hsl_mods.end(), shape->VertexCount(), hsl_mod);
	buf.rgb_mods.insert(buf.rgb_mods.end(), shape->VertexCount(), rgb_mod);
}

void BlockType::FillBlockMesh(
	BlockMesh::Buffer &buf,
	const glm::mat4 &transform,
	BlockMesh::Index idx_offset
) const noexcept {
	if (!shape) return;
	shape->Fill(buf, transform, textures, idx_offset);
	buf.hsl_mods.insert(buf.hsl_mods.end(), shape->VertexCount(), hsl_mod);
	buf.rgb_mods.insert(buf.rgb_mods.end(), shape->VertexCount(), rgb_mod);
}

void BlockType::OutlinePrimitiveMesh(PrimitiveMesh::Buffer &buf) const noexcept {
	if (!shape) return;
	shape->Outline(buf);
	buf.colors.insert(buf.colors.end(), shape->OutlineCount(), glm::vec4(outline_color, 1.0f));
}


BlockTypeRegistry::BlockTypeRegistry() {
	BlockType air;
	air.visible = false;
	air.block_light = false;
	air.collision = false;
	air.collide_block = false;
	Add(air);
}

Block::Type BlockTypeRegistry::Add(const BlockType &t) {
	int id = types.size();
	types.push_back(t);
	types.back().id = id;
	return id;
}


const glm::mat4 Block::orient2transform[ORIENT_COUNT] = {
	{  1,  0,  0,  0,  0,  1,  0,  0,  0,  0,  1,  0,  0,  0,  0,  1, }, // face: up,    turn: none
	{  0,  0, -1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  1, }, // face: up,    turn: left
	{ -1,  0,  0,  0,  0,  1,  0,  0,  0,  0, -1,  0,  0,  0,  0,  1, }, // face: up,    turn: around
	{  0,  0,  1,  0,  0,  1,  0,  0, -1,  0,  0,  0,  0,  0,  0,  1, }, // face: up,    turn: right
	{  1,  0,  0,  0,  0, -1,  0,  0,  0,  0, -1,  0,  0,  0,  0,  1, }, // face: down,  turn: none
	{  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0,  0,  0,  0,  0,  1, }, // face: down,  turn: left
	{ -1,  0,  0,  0,  0, -1,  0,  0,  0,  0,  1,  0,  0,  0,  0,  1, }, // face: down,  turn: around
	{  0,  0,  1,  0,  0, -1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  1, }, // face: down,  turn: right
	{  0, -1,  0,  0,  1,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  1, }, // face: right, turn: none
	{  0, -1,  0,  0,  0,  0, -1,  0,  1,  0,  0,  0,  0,  0,  0,  1, }, // face: right, turn: left
	{  0, -1,  0,  0, -1,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  1, }, // face: right, turn: around
	{  0, -1,  0,  0,  0,  0,  1,  0, -1,  0,  0,  0,  0,  0,  0,  1, }, // face: right, turn: right
	{  0,  1,  0,  0, -1,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  1, }, // face: left,  turn: none
	{  0,  1,  0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0,  0,  1, }, // face: left,  turn: left
	{  0,  1,  0,  0,  1,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  1, }, // face: left,  turn: around
	{  0,  1,  0,  0,  0,  0, -1,  0, -1,  0,  0,  0,  0,  0,  0,  1, }, // face: left,  turn: right
	{  1,  0,  0,  0,  0,  0,  1,  0,  0, -1,  0,  0,  0,  0,  0,  1, }, // face: front, turn: none
	{  0,  0, -1,  0,  1,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  1, }, // face: front, turn: left
	{ -1,  0,  0,  0,  0,  0, -1,  0,  0, -1,  0,  0,  0,  0,  0,  1, }, // face: front, turn: around
	{  0,  0,  1,  0, -1,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  1, }, // face: front, turn: right
	{  1,  0,  0,  0,  0,  0, -1,  0,  0,  1,  0,  0,  0,  0,  0,  1, }, // face: back,  turn: none
	{  0,  0, -1,  0, -1,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  1, }, // face: back,  turn: left
	{ -1,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  1, }, // face: back,  turn: around
	{  0,  0,  1,  0,  1,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  1, }, // face: back,  turn: right
};

const glm::ivec3 Block::face2normal[FACE_COUNT] = {
	{  0,  1,  0 },
	{  0, -1,  0 },
	{  1,  0,  0 },
	{ -1,  0,  0 },
	{  0,  0,  1 },
	{  0,  0, -1 },
};

const Block::Face Block::orient2face[ORIENT_COUNT][FACE_COUNT] = {
	{ FACE_UP,    FACE_DOWN,  FACE_RIGHT, FACE_LEFT,  FACE_FRONT, FACE_BACK,  }, // face: up,    turn: none
	{ FACE_UP,    FACE_DOWN,  FACE_FRONT, FACE_BACK,  FACE_LEFT,  FACE_RIGHT, }, // face: up,    turn: left
	{ FACE_UP,    FACE_DOWN,  FACE_LEFT,  FACE_RIGHT, FACE_BACK,  FACE_FRONT, }, // face: up,    turn: around
	{ FACE_UP,    FACE_DOWN,  FACE_BACK,  FACE_FRONT, FACE_RIGHT, FACE_LEFT,  }, // face: up,    turn: right
	{ FACE_DOWN,  FACE_UP,    FACE_RIGHT, FACE_LEFT,  FACE_BACK,  FACE_FRONT, }, // face: down,  turn: none
	{ FACE_DOWN,  FACE_UP,    FACE_BACK,  FACE_FRONT, FACE_LEFT,  FACE_RIGHT, }, // face: down,  turn: left
	{ FACE_DOWN,  FACE_UP,    FACE_LEFT,  FACE_RIGHT, FACE_FRONT, FACE_BACK,  }, // face: down,  turn: around
	{ FACE_DOWN,  FACE_UP,    FACE_FRONT, FACE_BACK,  FACE_RIGHT, FACE_LEFT,  }, // face: down,  turn: right
	{ FACE_LEFT,  FACE_RIGHT, FACE_UP,    FACE_DOWN,  FACE_FRONT, FACE_BACK,  }, // face: right, turn: none
	{ FACE_LEFT,  FACE_RIGHT, FACE_FRONT, FACE_BACK,  FACE_DOWN,  FACE_UP,    }, // face: right, turn: left
	{ FACE_LEFT,  FACE_RIGHT, FACE_DOWN,  FACE_UP,    FACE_BACK,  FACE_FRONT, }, // face: right, turn: around
	{ FACE_LEFT,  FACE_RIGHT, FACE_BACK,  FACE_FRONT, FACE_UP,    FACE_DOWN,  }, // face: right, turn: right
	{ FACE_RIGHT, FACE_LEFT,  FACE_DOWN,  FACE_UP,    FACE_FRONT, FACE_BACK,  }, // face: left,  turn: none
	{ FACE_RIGHT, FACE_LEFT,  FACE_FRONT, FACE_BACK,  FACE_UP,    FACE_DOWN,  }, // face: left,  turn: left
	{ FACE_RIGHT, FACE_LEFT,  FACE_UP,    FACE_DOWN,  FACE_BACK,  FACE_FRONT, }, // face: left,  turn: around
	{ FACE_RIGHT, FACE_LEFT,  FACE_BACK,  FACE_FRONT, FACE_DOWN,  FACE_UP,    }, // face: left,  turn: right
	{ FACE_BACK,  FACE_FRONT, FACE_RIGHT, FACE_LEFT,  FACE_UP,    FACE_DOWN,  }, // face: front, turn: none
	{ FACE_BACK,  FACE_FRONT, FACE_UP,    FACE_DOWN,  FACE_LEFT,  FACE_RIGHT, }, // face: front, turn: left
	{ FACE_BACK,  FACE_FRONT, FACE_LEFT,  FACE_RIGHT, FACE_DOWN,  FACE_UP,    }, // face: front, turn: around
	{ FACE_BACK,  FACE_FRONT, FACE_DOWN,  FACE_UP,    FACE_RIGHT, FACE_LEFT,  }, // face: front, turn: right
	{ FACE_FRONT, FACE_BACK,  FACE_RIGHT, FACE_LEFT,  FACE_DOWN,  FACE_UP,    }, // face: back,  turn: none
	{ FACE_FRONT, FACE_BACK,  FACE_DOWN,  FACE_UP,    FACE_LEFT,  FACE_RIGHT, }, // face: back,  turn: left
	{ FACE_FRONT, FACE_BACK,  FACE_LEFT,  FACE_RIGHT, FACE_UP,    FACE_DOWN,  }, // face: back,  turn: around
	{ FACE_FRONT, FACE_BACK,  FACE_UP,    FACE_DOWN,  FACE_RIGHT, FACE_LEFT,  }, // face: back,  turn: right
};

}
