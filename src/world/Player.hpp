#ifndef BLANK_WORLD_PLAYER_HPP_
#define BLANK_WORLD_PLAYER_HPP_

#include "Entity.hpp"


namespace blank {

class ChunkIndex;

class Player {

public:
	Player(Entity &e, ChunkIndex &i);
	~Player();

	Entity &GetEntity() const noexcept { return entity; }
	const std::string &Name() const noexcept { return entity.Name(); }
	Ray Aim() const { return entity.Aim(entity.ChunkCoords()); }

	ChunkIndex &GetChunks() const noexcept { return chunks; }

	void SetInventorySlot(int i) noexcept { inv_slot = i; }
	int GetInventorySlot() const noexcept { return inv_slot; }

	void Update(int dt);

private:
	Entity &entity;
	ChunkIndex &chunks;
	int inv_slot;

};

}

#endif
