#include "ChunkRenderer.hpp"

#include "World.hpp"
#include "../app/Assets.hpp"
#include "../graphics/BlockLighting.hpp"
#include "../graphics/Viewport.hpp"


namespace blank {

ChunkRenderer::ChunkRenderer(World &world, int rd)
: world(world)
, block_tex()
, render_dist(rd)
, side_length(2 * rd + 1)
, total_length(side_length * side_length * side_length)
, total_indexed(0)
, stride(1, side_length, side_length * side_length)
, models(total_length)
, chunks(total_length)
, base(0, 0, 0)
, fog_density(0.0f) {

}


void ChunkRenderer::LoadTextures(const AssetLoader &loader, const TextureIndex &tex_index) {
	block_tex.Bind();
	loader.LoadTextures(tex_index, block_tex);
	block_tex.FilterNearest();
}


bool ChunkRenderer::InRange(const Chunk::Pos &pos) const noexcept {
	return manhattan_radius(pos - base) <= render_dist;
}

int ChunkRenderer::IndexOf(const Chunk::Pos &pos) const noexcept {
	Chunk::Pos mod_pos(
		GetCol(pos.x),
		GetCol(pos.y),
		GetCol(pos.z)
	);
	return mod_pos.x * stride.x
		+  mod_pos.y * stride.y
		+  mod_pos.z * stride.z;
}


void ChunkRenderer::Rebase(const Chunk::Pos &new_base) {
	if (new_base == base) return;

	Chunk::Pos diff(new_base - base);

	if (manhattan_radius(diff) > render_dist) {
		// that's more than half, so probably not worth shifting
		base = new_base;
		Rescan();
		return;
	}

	while (diff.x > 0) {
		Shift(Block::FACE_RIGHT);
		--diff.x;
	}
	while (diff.x < 0) {
		Shift(Block::FACE_LEFT);
		++diff.x;
	}
	while (diff.y > 0) {
		Shift(Block::FACE_UP);
		--diff.y;
	}
	while (diff.y < 0) {
		Shift(Block::FACE_DOWN);
		++diff.y;
	}
	while (diff.z > 0) {
		Shift(Block::FACE_FRONT);
		--diff.z;
	}
	while (diff.z < 0) {
		Shift(Block::FACE_BACK);
		++diff.z;
	}
}

int ChunkRenderer::GetCol(int c) const noexcept {
	c %= side_length;
	if (c < 0) c += side_length;
	return c;
}

void ChunkRenderer::Shift(Block::Face f) {
	int a_axis = Block::Axis(f);
	int b_axis = (a_axis + 1) % 3;
	int c_axis = (a_axis + 2) % 3;
	int dir = Block::Direction(f);
	base[a_axis] += dir;
	int a = GetCol(base[a_axis] + (render_dist * dir));
	int a_stride = a * stride[a_axis];
	for (int b = 0; b < side_length; ++b) {
		int b_stride = b * stride[b_axis];
		for (int c = 0; c < side_length; ++c) {
			int bc_stride = b_stride + c * stride[c_axis];
			int index = a_stride + bc_stride;
			if (chunks[index]) {
				chunks[index] = nullptr;
				--total_indexed;
			}
			int neighbor = ((a - dir + side_length) % side_length) * stride[a_axis] + bc_stride;
			if (chunks[neighbor] && chunks[neighbor]->HasNeighbor(f)) {
				chunks[index] = &chunks[neighbor]->GetNeighbor(f);
				chunks[index]->InvalidateModel();
				++total_indexed;
			}
		}
	}
}


void ChunkRenderer::Rescan() {
	chunks.assign(total_length, nullptr);
	total_indexed = 0;
	Scan();
}

void ChunkRenderer::Scan() {
	for (Chunk &chunk : world.Loader().Loaded()) {
		if (!InRange(chunk.Position())) continue;
		int index = IndexOf(chunk.Position());
		if (!chunks[index]) {
			chunks[index] = &chunk;
			chunk.InvalidateModel();
			++total_indexed;
		}
	}
}

void ChunkRenderer::Update(int dt) {
	if (MissingChunks()) {
		Scan();
	}

	// maximum of 1000 per second too high?
	for (int i = 0, updates = 0; i < total_length && updates < dt; ++i) {
		if (chunks[i] && chunks[i]->ShouldUpdateModel()) {
			chunks[i]->Update(models[i]);
			++updates;
		}
	}
}


void ChunkRenderer::Render(Viewport &viewport) {
	BlockLighting &chunk_prog = viewport.ChunkProgram();
	chunk_prog.SetTexture(block_tex);
	chunk_prog.SetFogDensity(fog_density);

	for (int i = 0; i < total_length; ++i) {
		if (!chunks[i]) continue;
		glm::mat4 m(chunks[i]->Transform(base));
		glm::mat4 mvp(chunk_prog.GetVP() * m);
		if (!CullTest(Chunk::Bounds(), mvp)) {
			if (chunks[i]->ShouldUpdateModel()) {
				chunks[i]->Update(models[i]);
			}
			chunk_prog.SetM(m);
			models[i].Draw();
		}
	}
}

}
