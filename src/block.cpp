#include "block.hpp"

#include "geometry.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>


namespace blank {

namespace {

const glm::mat4 block_transforms[Block::DIR_COUNT * Block::ROT_COUNT] = {
	glm::mat4(1.0f),
	glm::eulerAngleY(PI_0p5),
	glm::eulerAngleY(PI),
	glm::eulerAngleY(PI_1p5),
	glm::eulerAngleX(PI),
	glm::eulerAngleYX(PI_0p5, PI),
	glm::eulerAngleYX(PI, PI),
	glm::eulerAngleYX(PI_1p5, PI),
	glm::eulerAngleZ(PI_0p5),
	glm::eulerAngleYZ(PI_0p5, PI_0p5),
	glm::eulerAngleYZ(PI, PI_0p5),
	glm::eulerAngleYZ(PI_1p5, PI_0p5),
	glm::eulerAngleZ(PI_1p5),
	glm::eulerAngleYZ(PI_0p5, PI_1p5),
	glm::eulerAngleYZ(PI, PI_1p5),
	glm::eulerAngleYZ(PI_1p5, PI_1p5),
	glm::eulerAngleX(PI_0p5),
	glm::eulerAngleYX(PI_0p5, PI_0p5),
	glm::eulerAngleYX(PI, PI_0p5),
	glm::eulerAngleYX(PI_1p5, PI_0p5),
	glm::eulerAngleX(PI_1p5),
	glm::eulerAngleYX(PI_0p5, PI_1p5),
	glm::eulerAngleYX(PI, PI_1p5),
	glm::eulerAngleYX(PI_1p5, PI_1p5),
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
, visible(v)
, fill({ false, false, false, false, false, false }) {

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
