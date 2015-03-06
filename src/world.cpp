#include "world.hpp"

#include <limits>
#include <glm/gtx/transform.hpp>


namespace blank {

const BlockType BlockType::DEFAULT;
const CuboidShape BlockType::DEFAULT_SHAPE({{ 0, 0, 0 }, { 1, 1, 1 }});

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


Chunk::Chunk()
: blocks(Size())
, model()
, transform(1.0f)
, dirty(false) {

}

Chunk::Chunk(Chunk &&other)
: blocks(std::move(other.blocks))
, model(std::move(other.model))
, transform(other.transform)
, dirty(other.dirty) {

}

Chunk &Chunk::operator =(Chunk &&other) {
	blocks = std::move(other.blocks);
	model = std::move(other.model);
	transform = other.transform;
	dirty = other.dirty;
	return *this;
}


void Chunk::Draw() {
	if (dirty) {
		Update();
	}
	model.Draw();
}


bool Chunk::Intersection(
	const Ray &ray,
	const glm::mat4 &M,
	int *blkid,
	float *dist,
	glm::vec3 *normal) const {
	{ // rough check
		const AABB bb{{0, 0, 0}, {Width(), Height(), Depth()}};
		if (!blank::Intersection(ray, bb, M)) {
			return false;
		}
	}

	if (!blkid && !dist && !normal) {
		return true;
	}

	// TODO: should be possible to heavily optimize this
	int id = 0;
	int closest_id = -1;
	float closest_dist = std::numeric_limits<float>::infinity();
	glm::vec3 closest_normal(0, 1, 0);
	for (int z = 0; z < Depth(); ++z) {
		for (int y = 0; y < Height(); ++y) {
			for (int x = 0; x < Width(); ++x, ++id) {
				if (!blocks[id].type->visible) {
					continue;
				}
				float cur_dist;
				glm::vec3 cur_norm;
				if (blocks[id].type->shape->Intersects(ray, glm::translate(M, glm::vec3(x, y, z)), cur_dist, cur_norm)) {
					if (cur_dist < closest_dist) {
						closest_id = id;
						closest_dist = cur_dist;
						closest_normal = cur_norm;
					}
				}
			}
		}
	}

	if (closest_id < 0) {
		return false;
	}

	if (blkid) {
		*blkid = closest_id;
	}
	if (dist) {
		*dist = closest_dist;
	}
	if (normal) {
		*normal = closest_normal;
	}
	return true;
}

void Chunk::Position(const glm::vec3 &pos) {
	position = pos;
	transform = glm::translate(pos * Extent());
}


int Chunk::VertexCount() const {
	// TODO: query blocks as soon as type shapes are implemented
	int count = 0;
	for (const auto &block : blocks) {
		count += block.type->shape->VertexCount();
	}
	return count;
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


World::World()
: blockType()
, blockShape({{ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }})
, slabShape({{ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.5f, 1.0f }})
, chunks() {
	blockType.Add(BlockType{ true, { 1.0f, 1.0f, 1.0f }, &blockShape }); // white block
	blockType.Add(BlockType{ true, { 1.0f, 1.0f, 1.0f }, &slabShape }); // white slab
	blockType.Add(BlockType{ true, { 1.0f, 0.0f, 0.0f }, &blockShape }); // red block
	blockType.Add(BlockType{ true, { 1.0f, 0.0f, 0.0f }, &slabShape }); // red slab
	blockType.Add(BlockType{ true, { 0.0f, 1.0f, 0.0f }, &blockShape }); // green block
	blockType.Add(BlockType{ true, { 0.0f, 1.0f, 0.0f }, &slabShape }); // green slab
	blockType.Add(BlockType{ true, { 0.0f, 0.0f, 1.0f }, &blockShape }); // blue block
	blockType.Add(BlockType{ true, { 0.0f, 0.0f, 1.0f }, &slabShape }); // blue slab
}


void World::Generate() {
	for (int z = -1; z < 2; ++z) {
		for (int y = -1; y < 2; ++y) {
			for (int x = -1; x < 2; ++x) {
				Generate(glm::vec3(x, y, z));
			}
		}
	}
}

Chunk &World::Generate(const glm::vec3 &pos) {
	chunks.emplace_back();
	Chunk &chunk = chunks.back();
	chunk.Position(pos);
	for (int i = 1; i < 9; ++i) {
		chunk.BlockAt(i) = Block(blockType[i]);
		chunk.BlockAt(i + 257) = Block(blockType[i]);
		chunk.BlockAt(i + 514) = Block(blockType[i]);
	}
	if (false) {
		chunk.BlockAt(glm::vec3(0, 0, 0)) = Block(blockType[4]);
		chunk.BlockAt(glm::vec3(0, 0, 1)) = Block(blockType[1]);
		chunk.BlockAt(glm::vec3(1, 0, 0)) = Block(blockType[5]);
		chunk.BlockAt(glm::vec3(1, 0, 1)) = Block(blockType[3]);
		chunk.BlockAt(glm::vec3(2, 0, 0)) = Block(blockType[4]);
		chunk.BlockAt(glm::vec3(2, 0, 1)) = Block(blockType[1]);
		chunk.BlockAt(glm::vec3(3, 0, 0)) = Block(blockType[2]);
		chunk.BlockAt(glm::vec3(3, 0, 1)) = Block(blockType[5]);
		chunk.BlockAt(glm::vec3(2, 0, 2)) = Block(blockType[4]);
		chunk.BlockAt(glm::vec3(2, 0, 3)) = Block(blockType[1]);
		chunk.BlockAt(glm::vec3(3, 0, 2)) = Block(blockType[2]);
		chunk.BlockAt(glm::vec3(3, 0, 3)) = Block(blockType[5]);
		chunk.BlockAt(glm::vec3(1, 1, 0)) = Block(blockType[5]);
		chunk.BlockAt(glm::vec3(1, 1, 1)) = Block(blockType[4]);
		chunk.BlockAt(glm::vec3(2, 1, 1)) = Block(blockType[3]);
		chunk.BlockAt(glm::vec3(2, 2, 1)) = Block(blockType[2]);
	}
	chunk.Invalidate();
	return chunk;
}

bool World::Intersection(
		const Ray &ray,
		const glm::mat4 &M,
		Chunk **chunk,
		int *blkid,
		float *dist,
		glm::vec3 *normal) {
	Chunk *closest_chunk = nullptr;
	int closest_blkid = -1;
	float closest_dist = std::numeric_limits<float>::infinity();
	glm::vec3 closest_normal;

	for (Chunk &cur_chunk : chunks) {
		int cur_blkid;
		float cur_dist;
		glm::vec3 cur_normal;
		if (cur_chunk.Intersection(ray, M * cur_chunk.Transform(), &cur_blkid, &cur_dist, &cur_normal)) {
			if (cur_dist < closest_dist) {
				closest_chunk = &cur_chunk;
				closest_blkid = cur_blkid;
				closest_dist = cur_dist;
				closest_normal = cur_normal;
			}
		}
	}

	if (chunk) {
		*chunk = closest_chunk;
	}
	if (blkid) {
		*blkid = closest_blkid;
	}
	if (dist) {
		*dist = closest_dist;
	}
	if (normal) {
		*normal = closest_normal;
	}
	return closest_chunk;
}


Chunk &World::Next(const Chunk &to, const glm::vec3 &dir) {
	const glm::vec3 tgt_pos = to.Position() + dir;
	for (Chunk &chunk : chunks) {
		if (chunk.Position() == tgt_pos) {
			return chunk;
		}
	}
	return Generate(tgt_pos);
}

}
