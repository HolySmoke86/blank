#include "world.hpp"

#include <limits>
#include <glm/gtx/transform.hpp>


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
				glm::vec3 pos(float(x) + 0.5f, float(y) + 0.5f, float(z) + 0.5f);
				if (blocks[id].type->shape->Intersects(ray, glm::translate(M, pos), cur_dist, cur_norm)) {
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
	int count = 0;
	for (const auto &block : blocks) {
		count += block.type->shape->VertexCount();
	}
	return count;
}

void Chunk::Update() {
	model.Clear();
	model.Reserve(VertexCount());

	for (size_t i = 0; i < Size(); ++i) {
		blocks[i].type->FillModel(ToCoords(i), model);
	}

	model.Invalidate();
	dirty = false;
}


World::World()
: blockType()
, blockShape({{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }})
, stairShape({{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }}, { 0.0f, 0.0f })
, slabShape({{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.0f, 0.5f }})
, blockNoise(0)
, colorNoise(1)
, chunks() {
	blockType.Add(BlockType{ true, { 1.0f, 1.0f, 1.0f }, &blockShape }); // white block
	blockType.Add(BlockType{ true, { 1.0f, 1.0f, 1.0f }, &stairShape }); // white stair
	blockType.Add(BlockType{ true, { 1.0f, 1.0f, 1.0f }, &slabShape }); // white slab
	blockType.Add(BlockType{ true, { 1.0f, 0.0f, 0.0f }, &blockShape }); // red block
	blockType.Add(BlockType{ true, { 1.0f, 0.0f, 0.0f }, &stairShape }); // red stair
	blockType.Add(BlockType{ true, { 1.0f, 0.0f, 0.0f }, &slabShape }); // red slab
	blockType.Add(BlockType{ true, { 0.0f, 1.0f, 0.0f }, &blockShape }); // green block
	blockType.Add(BlockType{ true, { 0.0f, 1.0f, 0.0f }, &stairShape }); // green stair
	blockType.Add(BlockType{ true, { 0.0f, 1.0f, 0.0f }, &slabShape }); // green slab
	blockType.Add(BlockType{ true, { 0.0f, 0.0f, 1.0f }, &blockShape }); // blue block
	blockType.Add(BlockType{ true, { 0.0f, 0.0f, 1.0f }, &stairShape }); // blue stair
	blockType.Add(BlockType{ true, { 0.0f, 0.0f, 1.0f }, &slabShape }); // blue slab

	player.Position({ 4.0f, 4.0f, 4.0f });
}


void World::Generate() {
	for (int z = -2; z < 3; ++z) {
		for (int y = -2; y < 3; ++y) {
			for (int x = -2; x < 3; ++x) {
				Generate(glm::vec3(x, y, z));
			}
		}
	}
}

Chunk &World::Generate(const glm::vec3 &pos) {
	chunks.emplace_back();
	Chunk &chunk = chunks.back();
	chunk.Position(pos);
	if (pos.x == 0 && pos.y == 0 && pos.z == 0) {
		for (size_t i = 1; i < blockType.Size(); ++i) {
			chunk.BlockAt(i) = Block(blockType[i]);
			chunk.BlockAt(i + 257) = Block(blockType[i]);
			chunk.BlockAt(i + 514) = Block(blockType[i]);
		}
	} else {
		for (int z = 0; z < Chunk::Depth(); ++z) {
			for (int y = 0; y < Chunk::Height(); ++y) {
				for (int x = 0; x < Chunk::Width(); ++x) {
					glm::vec3 block_pos{float(x), float(y), float(z)};
					glm::vec3 gen_pos = (pos * Chunk::Extent() + block_pos) / 64.0f;
					float val = blockNoise(gen_pos);
					if (val > 0.8f) {
						int col_val = int((colorNoise(gen_pos) + 1.0f) * 2.0f) % 4;
						chunk.BlockAt(block_pos) = Block(blockType[col_val * 3 + 1]);
					}
				}
			}
		}
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


void World::Update(int dt) {
	player.Update(dt);
}


void World::Render(DirectionalLighting &program) {
	program.SetLightDirection({ -1.0f, -3.0f, -2.0f });
	program.SetView(glm::inverse(player.Transform()));

	for (Chunk &chunk : LoadedChunks()) {
		program.SetM(chunk.Transform());
		chunk.Draw();
	}
}

}
