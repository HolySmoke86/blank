#include "block.hpp"

#include "geometry.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>


namespace blank {

namespace {

const glm::mat4 block_transforms[Block::FACE_COUNT * Block::TURN_COUNT] = {
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

}

const glm::mat4 &Block::Transform() const {
	return block_transforms[orient];
}


const NullShape BlockType::DEFAULT_SHAPE;

BlockType::BlockType(bool v, const glm::vec3 &col, const Shape *s)
: shape(s)
, color(col)
, outline_color(-1, -1, -1)
, id(0)
, luminosity(0)
, visible(v)
, block_light(false)
, fill({ false, false, false, false, false, false }) {

}

namespace {

const Block::Face face_map[Block::FACE_COUNT * Block::TURN_COUNT][Block::FACE_COUNT] = {
	{ Block::FACE_UP,    Block::FACE_DOWN,  Block::FACE_RIGHT, Block::FACE_LEFT,  Block::FACE_FRONT, Block::FACE_BACK,  }, // face: up,    turn: none x
	{ Block::FACE_UP,    Block::FACE_DOWN,  Block::FACE_FRONT, Block::FACE_BACK,  Block::FACE_LEFT,  Block::FACE_RIGHT, }, // face: up,    turn: left
	{ Block::FACE_UP,    Block::FACE_DOWN,  Block::FACE_LEFT,  Block::FACE_RIGHT, Block::FACE_BACK,  Block::FACE_FRONT, }, // face: up,    turn: around
	{ Block::FACE_UP,    Block::FACE_DOWN,  Block::FACE_BACK,  Block::FACE_FRONT, Block::FACE_RIGHT, Block::FACE_LEFT,  }, // face: up,    turn: right
	{ Block::FACE_DOWN,  Block::FACE_UP,    Block::FACE_RIGHT, Block::FACE_LEFT,  Block::FACE_BACK,  Block::FACE_FRONT, }, // face: down,  turn: none
	{ Block::FACE_DOWN,  Block::FACE_UP,    Block::FACE_BACK,  Block::FACE_FRONT, Block::FACE_LEFT,  Block::FACE_RIGHT, }, // face: down,  turn: left
	{ Block::FACE_DOWN,  Block::FACE_UP,    Block::FACE_LEFT,  Block::FACE_RIGHT, Block::FACE_FRONT, Block::FACE_BACK,  }, // face: down,  turn: around
	{ Block::FACE_DOWN,  Block::FACE_UP,    Block::FACE_FRONT, Block::FACE_BACK,  Block::FACE_RIGHT, Block::FACE_LEFT,  }, // face: down,  turn: right
	{ Block::FACE_LEFT,  Block::FACE_RIGHT, Block::FACE_UP,    Block::FACE_DOWN,  Block::FACE_FRONT, Block::FACE_BACK,  }, // face: right, turn: none
	{ Block::FACE_LEFT,  Block::FACE_RIGHT, Block::FACE_FRONT, Block::FACE_BACK,  Block::FACE_DOWN,  Block::FACE_UP,    }, // face: right, turn: left
	{ Block::FACE_LEFT,  Block::FACE_RIGHT, Block::FACE_DOWN,  Block::FACE_UP,    Block::FACE_BACK,  Block::FACE_FRONT, }, // face: right, turn: around
	{ Block::FACE_LEFT,  Block::FACE_RIGHT, Block::FACE_BACK,  Block::FACE_FRONT, Block::FACE_UP,    Block::FACE_DOWN,  }, // face: right, turn: right
	{ Block::FACE_RIGHT, Block::FACE_LEFT,  Block::FACE_DOWN,  Block::FACE_UP,    Block::FACE_FRONT, Block::FACE_BACK,  }, // face: left,  turn: none
	{ Block::FACE_RIGHT, Block::FACE_LEFT,  Block::FACE_FRONT, Block::FACE_BACK,  Block::FACE_UP,    Block::FACE_DOWN,  }, // face: left,  turn: left
	{ Block::FACE_RIGHT, Block::FACE_LEFT,  Block::FACE_UP,    Block::FACE_DOWN,  Block::FACE_BACK,  Block::FACE_FRONT, }, // face: left,  turn: around
	{ Block::FACE_RIGHT, Block::FACE_LEFT,  Block::FACE_BACK,  Block::FACE_FRONT, Block::FACE_DOWN,  Block::FACE_UP,    }, // face: left,  turn: right
	{ Block::FACE_BACK,  Block::FACE_FRONT, Block::FACE_RIGHT, Block::FACE_LEFT,  Block::FACE_UP,    Block::FACE_DOWN,  }, // face: front, turn: none
	{ Block::FACE_BACK,  Block::FACE_FRONT, Block::FACE_UP,    Block::FACE_DOWN,  Block::FACE_LEFT,  Block::FACE_RIGHT, }, // face: front, turn: left
	{ Block::FACE_BACK,  Block::FACE_FRONT, Block::FACE_LEFT,  Block::FACE_RIGHT, Block::FACE_DOWN,  Block::FACE_UP,    }, // face: front, turn: around
	{ Block::FACE_BACK,  Block::FACE_FRONT, Block::FACE_DOWN,  Block::FACE_UP,    Block::FACE_RIGHT, Block::FACE_LEFT,  }, // face: front, turn: right
	{ Block::FACE_FRONT, Block::FACE_BACK,  Block::FACE_RIGHT, Block::FACE_LEFT,  Block::FACE_DOWN,  Block::FACE_UP,    }, // face: back,  turn: none
	{ Block::FACE_FRONT, Block::FACE_BACK,  Block::FACE_DOWN,  Block::FACE_UP,    Block::FACE_LEFT,  Block::FACE_RIGHT, }, // face: back,  turn: left
	{ Block::FACE_FRONT, Block::FACE_BACK,  Block::FACE_LEFT,  Block::FACE_RIGHT, Block::FACE_UP,    Block::FACE_DOWN,  }, // face: back,  turn: around
	{ Block::FACE_FRONT, Block::FACE_BACK,  Block::FACE_UP,    Block::FACE_DOWN,  Block::FACE_RIGHT, Block::FACE_LEFT,  }, // face: back,  turn: right
};

}

bool BlockType::FaceFilled(const Block &block, Block::Face face) const {
	return fill[face_map[block.orient][face]];
}

void BlockType::FillModel(
	Model::Buffer &buf,
	const glm::mat4 &transform,
	Model::Index idx_offset
) const {
	shape->Vertices(buf.vertices, buf.normals, buf.indices, transform, idx_offset);
	buf.colors.insert(buf.colors.end(), shape->VertexCount(), color);
}

void BlockType::FillOutlineModel(
	OutlineModel &model,
	const glm::vec3 &pos_offset,
	OutlineModel::Index idx_offset
) const {
	shape->Outline(model.vertices, model.indices, pos_offset, idx_offset);
	model.colors.insert(model.colors.end(), shape->OutlineCount(), outline_color);
}


BlockTypeRegistry::BlockTypeRegistry() {
	Add(BlockType());
}

Block::Type BlockTypeRegistry::Add(const BlockType &t) {
	int id = types.size();
	types.push_back(t);
	types.back().id = id;
	return id;
}

}
