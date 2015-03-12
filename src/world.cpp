#include "world.hpp"

#include <limits>
#include <glm/gtx/transform.hpp>


namespace blank {

World::World()
: blockType()
, blockShape({{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }})
, stairShape({{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }}, { 0.0f, 0.0f })
, slabShape({{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.0f, 0.5f }})
, generate(0)
, chunks(blockType, generate)
, player() {
	BlockType::Faces block_fill = {  true,  true,  true,  true,  true,  true };
	BlockType::Faces slab_fill  = { false,  true, false, false, false, false };
	BlockType::Faces stair_fill = { false,  true, false, false, false,  true };

	{ // white block
		BlockType type(true, { 1.0f, 1.0f, 1.0f }, &blockShape);
		type.fill = block_fill;
		blockType.Add(type);
	}
	{ // white slab
		BlockType type(true, { 1.0f, 1.0f, 1.0f }, &slabShape);
		type.fill = slab_fill;
		blockType.Add(type);
	}
	{ // white stair
		BlockType type(true, { 1.0f, 1.0f, 1.0f }, &stairShape);
		type.fill = stair_fill;
		blockType.Add(type);
	}

	{ // red block
		BlockType type(true, { 1.0f, 0.0f, 0.0f }, &blockShape);
		type.fill = block_fill;
		blockType.Add(type);
	}
	{ // red slab
		BlockType type(true, { 1.0f, 0.0f, 0.0f }, &slabShape);
		type.fill = slab_fill;
		blockType.Add(type);
	}
	{ // red stair
		BlockType type(true, { 1.0f, 0.0f, 0.0f }, &stairShape);
		type.fill = stair_fill;
		blockType.Add(type);
	}

	{ // green block
		BlockType type(true, { 0.0f, 1.0f, 0.0f }, &blockShape);
		type.fill = block_fill;
		blockType.Add(type);
	}
	{ // green slab
		BlockType type(true, { 0.0f, 1.0f, 0.0f }, &slabShape);
		type.fill = slab_fill;
		blockType.Add(type);
	}
	{ // green stair
		BlockType type(true, { 0.0f, 1.0f, 0.0f }, &stairShape);
		type.fill = stair_fill;
		blockType.Add(type);
	}

	{ // blue block
		BlockType type(true, { 0.0f, 0.0f, 1.0f }, &blockShape);
		type.fill = block_fill;
		blockType.Add(type);
	}
	{ // blue slab
		BlockType type(true, { 0.0f, 0.0f, 1.0f }, &slabShape);
		type.fill = slab_fill;
		blockType.Add(type);
	}
	{ // blue stair
		BlockType type(true, { 0.0f, 0.0f, 1.0f }, &stairShape);
		type.fill = stair_fill;
		blockType.Add(type);
	}

	generate.Space(0);
	generate.Solids({ 1, 4, 7, 10 });

	player.Position({ 4.0f, 4.0f, 4.0f });

	chunks.Generate({ -4, -4, -4 }, { 5, 5, 5});
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

	for (Chunk &cur_chunk : chunks.Loaded()) {
		int cur_blkid;
		float cur_dist;
		glm::vec3 cur_normal;
		if (cur_chunk.Intersection(ray, M * cur_chunk.Transform(player.ChunkCoords()), &cur_blkid, &cur_dist, &cur_normal)) {
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


Chunk &World::Next(const Chunk &to, const glm::tvec3<int> &dir) {
	const Chunk::Pos tgt_pos = to.Position() + dir;
	return chunks.ForceLoad(tgt_pos);
}


void World::Update(int dt) {
	player.Update(dt);
	chunks.Rebase(player.ChunkCoords());
	chunks.Update();
}


void World::Render(DirectionalLighting &program) {
	program.SetLightDirection({ -1.0f, -3.0f, -2.0f });
	program.SetView(glm::inverse(player.Transform(player.ChunkCoords())));

	for (Chunk &chunk : chunks.Loaded()) {
		glm::mat4 m(chunk.Transform(player.ChunkCoords()));
		program.SetM(m);
		glm::mat4 mvp(program.GetVP() * m);
		if (!CullTest(Chunk::Bounds(), mvp)) {
			chunk.Draw();
		}
	}
}

}
