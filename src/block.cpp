#include "block.hpp"


namespace blank {

const BlockType BlockType::DEFAULT;
const NullShape BlockType::DEFAULT_SHAPE;

void BlockType::FillVBO(
	const glm::vec3 &pos,
	std::vector<glm::vec3> &vertices,
	std::vector<glm::vec3> &colors,
	std::vector<glm::vec3> &normals
) const {
	shape->Vertices(vertices, pos);
	colors.insert(colors.end(), shape->VertexCount(), color);
	shape->Normals(normals);
}

void BlockType::FillOutlineVBO(
	std::vector<glm::vec3> &vertices,
	std::vector<glm::vec3> &colors
) const {
	shape->Outline(vertices);
	colors.insert(colors.end(), shape->OutlineCount(), outline_color);
}


BlockTypeRegistry::BlockTypeRegistry() {
	Add(BlockType::DEFAULT);
}

int BlockTypeRegistry::Add(const BlockType &t) {
	int id = types.size();
	types.push_back(t);
	types.back().id = id;
	return id;
}

}
