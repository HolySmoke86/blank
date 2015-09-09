#ifndef BLANK_WORLD_PLAYER_HPP_
#define BLANK_WORLD_PLAYER_HPP_

namespace blank {

class ChunkIndex;
class Entity;

struct Player {

	Entity *entity;
	ChunkIndex *chunks;

	Player(Entity *e, ChunkIndex *i)
	: entity(e), chunks(i) { }

};

}

#endif
