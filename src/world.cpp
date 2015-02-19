#include "world.hpp"


namespace blank {

const BlockType BlockType::DEFAULT;

void BlockType::FillVBO(
	const glm::vec3 &pos,
	std::vector<glm::vec3> &vertices,
	std::vector<glm::vec3> &colors,
	std::vector<glm::vec3> &normals
) const {
	vertices.emplace_back(pos.x    , pos.y    , pos.z + 1); // front
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y    , pos.z    ); // back
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z    );
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z    ); // top
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y    , pos.z    ); // bottom
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z    );
	vertices.emplace_back(pos.x    , pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y    , pos.z    ); // left
	vertices.emplace_back(pos.x    , pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x    , pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x    , pos.y + 1, pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z    ); // right
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y    , pos.z + 1);
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z    );
	vertices.emplace_back(pos.x + 1, pos.y + 1, pos.z + 1);

	colors.insert(colors.end(), 6, glm::vec3(1.0f, 1.0f, 1.0f)); // front
	colors.insert(colors.end(), 6, glm::vec3(1.0f, 1.0f, 1.0f)); // back
	colors.insert(colors.end(), 6, glm::vec3(1.0f, 1.0f, 1.0f)); // top
	colors.insert(colors.end(), 6, glm::vec3(1.0f, 1.0f, 1.0f)); // bottom
	colors.insert(colors.end(), 6, glm::vec3(1.0f, 1.0f, 1.0f)); // left
	colors.insert(colors.end(), 6, glm::vec3(1.0f, 1.0f, 1.0f)); // right

	normals.insert(normals.end(), 6, glm::vec3( 0.0f,  0.0f,  1.0f)); // front
	normals.insert(normals.end(), 6, glm::vec3( 0.0f,  0.0f, -1.0f)); // back
	normals.insert(normals.end(), 6, glm::vec3( 0.0f,  1.0f,  0.0f)); // top
	normals.insert(normals.end(), 6, glm::vec3( 0.0f, -1.0f,  0.0f)); // bottom
	normals.insert(normals.end(), 6, glm::vec3(-1.0f,  0.0f,  0.0f)); // left
	normals.insert(normals.end(), 6, glm::vec3( 1.0f,  0.0f,  0.0f)); // right
}


Chunk::Chunk()
: blocks(Size())
, model()
, dirty(false) {

}


void Chunk::Draw() {
	if (dirty) {
		Update();
	}
	model.Draw();
}


int Chunk::VertexCount() const {
	// TODO: query blocks as soon as type shapes are implemented
	return Size() * 6 * 6;
}

void Chunk::Update() {
	model.Clear();
	model.Reserve(VertexCount());

	for (int i = 0; i < Size(); ++i) {
		if (blocks[i].type->visible) {
			blocks[i].type->FillModel(ToCoords(i), model);
		}
	}

	model.Invalidate();
	dirty = false;
}

}
