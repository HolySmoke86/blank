#include "World.hpp"

#include "../graphics/BlockLighting.hpp"
#include "../graphics/DirectionalLighting.hpp"

#include <iostream>
#include <limits>
#include <glm/gtx/transform.hpp>


namespace blank {

World::World(const Config &config)
: blockType()
, blockShape({{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }})
, stairShape({{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f }}, { 0.0f, 0.0f })
, slabShape({{ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.0f, 0.5f }})
, generate(config.gen)
, chunks(config.load, blockType, generate)
, player()
, entities()
, light_direction(config.light_direction)
, fog_density(config.fog_density) {
	BlockType::Faces block_fill = {  true,  true,  true,  true,  true,  true };
	BlockType::Faces slab_fill  = { false,  true, false, false, false, false };
	BlockType::Faces stair_fill = { false,  true, false, false, false,  true };

	{ // white block
		BlockType type(true, { 1.0f, 1.0f, 1.0f }, &blockShape);
		type.block_light = true;
		type.fill = block_fill;
		blockType.Add(type);
	}
	{ // white slab
		BlockType type(true, { 1.0f, 1.0f, 1.0f }, &slabShape);
		type.block_light = true;
		type.fill = slab_fill;
		blockType.Add(type);
	}
	{ // white stair
		BlockType type(true, { 1.0f, 1.0f, 1.0f }, &stairShape);
		type.block_light = true;
		type.fill = stair_fill;
		blockType.Add(type);
	}

	{ // red block
		BlockType type(true, { 1.0f, 0.0f, 0.0f }, &blockShape);
		type.block_light = true;
		type.fill = block_fill;
		blockType.Add(type);
	}
	{ // red slab
		BlockType type(true, { 1.0f, 0.0f, 0.0f }, &slabShape);
		type.block_light = true;
		type.fill = slab_fill;
		blockType.Add(type);
	}
	{ // red stair
		BlockType type(true, { 1.0f, 0.0f, 0.0f }, &stairShape);
		type.block_light = true;
		type.fill = stair_fill;
		blockType.Add(type);
	}

	{ // green block
		BlockType type(true, { 0.0f, 1.0f, 0.0f }, &blockShape);
		type.block_light = true;
		type.fill = block_fill;
		blockType.Add(type);
	}
	{ // green slab
		BlockType type(true, { 0.0f, 1.0f, 0.0f }, &slabShape);
		type.block_light = true;
		type.fill = slab_fill;
		blockType.Add(type);
	}
	{ // green stair
		BlockType type(true, { 0.0f, 1.0f, 0.0f }, &stairShape);
		type.block_light = true;
		type.fill = stair_fill;
		blockType.Add(type);
	}

	{ // blue block
		BlockType type(true, { 0.0f, 0.0f, 1.0f }, &blockShape);
		type.block_light = true;
		type.fill = block_fill;
		blockType.Add(type);
	}
	{ // blue slab
		BlockType type(true, { 0.0f, 0.0f, 1.0f }, &slabShape);
		type.block_light = true;
		type.fill = slab_fill;
		blockType.Add(type);
	}
	{ // blue stair
		BlockType type(true, { 0.0f, 0.0f, 1.0f }, &stairShape);
		type.block_light = true;
		type.fill = stair_fill;
		blockType.Add(type);
	}

	{ // glowing yellow block
		BlockType type(true, { 1.0f, 1.0f, 0.0f }, &blockShape);
		type.luminosity = 15;
		type.block_light = true;
		type.fill = block_fill;
		blockType.Add(type);
	}

	generate.Space(0);
	generate.Light(13);
	generate.Solids({ 1, 4, 7, 10 });

	player = &AddEntity();
	player->Name("player");
	player->Bounds({ { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } });
	player->WorldCollidable(true);
	player->Position(config.spawn);

	chunks.GenerateSurrounding(player->ChunkCoords());
}


namespace {

struct Candidate {
	Chunk *chunk;
	float dist;
};

std::vector<Candidate> candidates;

}

bool World::Intersection(
	const Ray &ray,
	const glm::mat4 &M,
	Chunk *&chunk,
	int &blkid,
	float &dist,
	glm::vec3 &normal
) {
	candidates.clear();

	for (Chunk &cur_chunk : chunks.Loaded()) {
		float cur_dist;
		if (cur_chunk.Intersection(ray, M * cur_chunk.Transform(player->ChunkCoords()), cur_dist)) {
			candidates.push_back({ &cur_chunk, cur_dist });
		}
	}

	if (candidates.empty()) return false;

	chunk = nullptr;
	dist = std::numeric_limits<float>::infinity();
	blkid = -1;

	for (Candidate &cand : candidates) {
		if (cand.dist > dist) continue;
		int cur_blkid;
		float cur_dist;
		glm::vec3 cur_normal;
		if (cand.chunk->Intersection(ray, M * cand.chunk->Transform(player->ChunkCoords()), cur_blkid, cur_dist, cur_normal)) {
			if (cur_dist < dist) {
				chunk = cand.chunk;
				blkid = cur_blkid;
				dist = cur_dist;
				normal = cur_normal;
			}
		}
	}

	return chunk;
}

bool World::Intersection(const Entity &e) {
	AABB box = e.Bounds();
	glm::mat4 M = e.Transform(player->ChunkCoords());
	// TODO: this only needs to check the chunks surrounding the entity's chunk position
	//       need find out if that is quicker than the rough chunk bounds test
	for (Chunk &cur_chunk : chunks.Loaded()) {
		if (cur_chunk.Intersection(box, M, cur_chunk.Transform(player->ChunkCoords()))) {
			return true;
		}
	}
	return false;
}


Chunk &World::PlayerChunk() {
	return chunks.ForceLoad(player->ChunkCoords());
}

Chunk &World::Next(const Chunk &to, const glm::tvec3<int> &dir) {
	const Chunk::Pos tgt_pos = to.Position() + dir;
	return chunks.ForceLoad(tgt_pos);
}


void World::Update(int dt) {
	for (Entity &entity : entities) {
		entity.Update(dt);
	}
	for (Entity &entity : entities) {
		if (entity.WorldCollidable() && Intersection(entity)) {
			// entity collides with the world
			std::cout << entity.Name() << " entity intersects world" << std::endl;
		}
	}
	chunks.Rebase(player->ChunkCoords());
	chunks.Update(dt);
}


void World::Render(BlockLighting &chunk_prog, DirectionalLighting &entity_prog) {
	chunk_prog.Activate();
	chunk_prog.SetFogDensity(fog_density);
	chunk_prog.SetView(glm::inverse(player->Transform(player->ChunkCoords())));

	for (Chunk &chunk : chunks.Loaded()) {
		glm::mat4 m(chunk.Transform(player->ChunkCoords()));
		chunk_prog.SetM(m);
		glm::mat4 mvp(chunk_prog.GetVP() * m);
		if (!CullTest(Chunk::Bounds(), mvp)) {
			chunk.Draw();
		}
	}

	entity_prog.Activate();
	entity_prog.SetLightDirection(light_direction);
	entity_prog.SetFogDensity(fog_density);
	entity_prog.SetView(glm::inverse(player->Transform(player->ChunkCoords())));

	for (Entity &entity : entities) {
		if (entity.HasShape()) {
			entity_prog.SetM(entity.Transform(player->ChunkCoords()));
			entity.Draw();
		}
	}
}

}
