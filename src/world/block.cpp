#include "Block.hpp"
#include "BlockType.hpp"
#include "BlockTypeRegistry.hpp"

#include "../model/geometry.hpp"

#include <ostream>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>


namespace blank {

const NullShape BlockType::DEFAULT_SHAPE;


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
: shape(&DEFAULT_SHAPE)
, texture(0)
, hsl_mod(0.0f, 1.0f, 1.0f)
, rgb_mod(1.0f, 1.0f, 1.0f)
, outline_color(-1, -1, -1)
, label("some block")
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
, commonness(1.0f)
, fill({ false, false, false, false, false, false }) {

}

void BlockType::FillEntityModel(
	EntityModel::Buffer &buf,
	const glm::mat4 &transform,
	EntityModel::Index idx_offset
) const noexcept {
	shape->Vertices(buf, transform, texture, idx_offset);
	buf.hsl_mods.insert(buf.hsl_mods.end(), shape->VertexCount(), hsl_mod);
	buf.rgb_mods.insert(buf.rgb_mods.end(), shape->VertexCount(), rgb_mod);
}

void BlockType::FillBlockModel(
	BlockModel::Buffer &buf,
	const glm::mat4 &transform,
	BlockModel::Index idx_offset
) const noexcept {
	shape->Vertices(buf, transform, texture, idx_offset);
	buf.hsl_mods.insert(buf.hsl_mods.end(), shape->VertexCount(), hsl_mod);
	buf.rgb_mods.insert(buf.rgb_mods.end(), shape->VertexCount(), rgb_mod);
}

void BlockType::FillOutlineModel(OutlineModel::Buffer &buf) const noexcept {
	shape->Outline(buf);
	buf.colors.insert(buf.colors.end(), shape->OutlineCount(), outline_color);
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
