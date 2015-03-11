#include "block.hpp"


namespace blank {

const NullShape BlockType::DEFAULT_SHAPE;

void BlockType::FillModel(
	Model::Buffer &buf,
	const glm::vec3 &pos_offset,
	Model::Index idx_offset
) const {
	shape->Vertices(buf.vertices, buf.normals, buf.indices, pos_offset, idx_offset);
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
