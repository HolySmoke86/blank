#ifndef BLANK_WORLD_WORLDMANIPULATOR_HPP_
#define BLANK_WORLD_WORLDMANIPULATOR_HPP_


namespace blank {

class Block;
class Chunk;

struct WorldManipulator {

	virtual void SetBlock(Chunk &, int, const Block &) = 0;

};

}

#endif
