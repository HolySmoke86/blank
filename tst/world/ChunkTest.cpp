#include "ChunkTest.hpp"

#include "world/BlockType.hpp"
#include "world/Chunk.hpp"

#include <memory>

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::ChunkTest);

using std::unique_ptr;


namespace blank {
namespace test {

void ChunkTest::setUp() {
	types = BlockTypeRegistry();

	BlockType obstacle;
	obstacle.visible = true;
	obstacle.block_light = true;
	types.Add(obstacle);

	BlockType source;
	source.visible = true;
	source.luminosity = 5;
	source.block_light = true;
	types.Add(source);
}

void ChunkTest::tearDown() {
}


void ChunkTest::testBounds() {
	CPPUNIT_ASSERT_MESSAGE(
		"valid position out of bounds",
		Chunk::InBounds(Chunk::Pos(0, 0, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"valid position out of bounds",
		Chunk::InBounds(Chunk::Pos(15, 0, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"valid position out of bounds",
		Chunk::InBounds(Chunk::Pos(0, 15, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"valid position out of bounds",
		Chunk::InBounds(Chunk::Pos(0, 0, 15))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"valid position out of bounds",
		Chunk::InBounds(Chunk::Pos(15, 15, 15))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"invalid position in bounds",
		!Chunk::InBounds(Chunk::Pos(-1, -1, -1))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"invalid position in bounds",
		!Chunk::InBounds(Chunk::Pos(-1, 1, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"invalid position in bounds",
		!Chunk::InBounds(Chunk::Pos(16, -16, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"invalid position in bounds",
		!Chunk::InBounds(Chunk::Pos(16, 16, 16))
	);
}

void ChunkTest::testBorder() {
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(0, 0, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(0, 0, 8))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(0, 0, 15))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(0, 8, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(0, 8, 8))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(0, 8, 15))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(0, 15, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(0, 15, 8))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(0, 15, 15))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(8, 0, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(8, 0, 8))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(8, 0, 15))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(8, 8, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position is border",
		!Chunk::IsBorder(Chunk::Pos(8, 8, 8))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(8, 8, 15))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(8, 15, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(8, 15, 8))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(8, 15, 15))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(15, 0, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(15, 0, 8))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(15, 0, 15))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(15, 8, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(15, 8, 8))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(15, 8, 15))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(15, 15, 0))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(15, 15, 8))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::Pos(15, 15, 15))
	);

	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(0, 0, 0)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(0, 0, 8)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(0, 0, 15)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(0, 8, 0)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(0, 8, 8)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(0, 8, 15)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(0, 15, 0)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(0, 15, 8)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(0, 15, 15)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(8, 0, 0)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(8, 0, 8)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(8, 0, 15)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(8, 8, 0)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position is border",
		!Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(8, 8, 8)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(8, 8, 15)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(8, 15, 0)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(8, 15, 8)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(8, 15, 15)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(15, 0, 0)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(15, 0, 8)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(15, 0, 15)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(15, 8, 0)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(15, 8, 8)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(15, 8, 15)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(15, 15, 0)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(15, 15, 8)))
	);
	CPPUNIT_ASSERT_MESSAGE(
		"position not border",
		Chunk::IsBorder(Chunk::ToIndex(Chunk::Pos(15, 15, 15)))
	);
}

void ChunkTest::testNeighbor() {
	unique_ptr<Chunk> chunk(new Chunk(types));
	chunk->Position({0, 0, 0});
	for (int i = 0; i < Block::FACE_COUNT; ++i) {
		CPPUNIT_ASSERT_MESSAGE(
			"sole chunk has neighbor",
			!chunk->HasNeighbor(Block::Face(i))
		);
	}

	unique_ptr<Chunk> neighbor(new Chunk(types));
	for (int i = 0; i < Block::FACE_COUNT; ++i) {
		Block::Face face = Block::Face(i);
		neighbor->Position(Block::FaceNormal(face));
		chunk->SetNeighbor(*neighbor);
		CPPUNIT_ASSERT_MESSAGE(
			"chunk did not link right neighbor",
			chunk->HasNeighbor(face)
		);
		CPPUNIT_ASSERT_MESSAGE(
			"chunk did not link right neighbor",
			neighbor->HasNeighbor(Block::Opposite(face))
		);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"chunk did not link correct neighbor",
			&*neighbor, &chunk->GetNeighbor(face)
		);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"chunk did not link correct neighbor",
			&*chunk, &neighbor->GetNeighbor(Block::Opposite(face))
		);
		chunk->ClearNeighbors();
	}

	neighbor->Position({1, 1, 1});
	chunk->SetNeighbor(*neighbor);
	for (int i = 0; i < Block::FACE_COUNT; ++i) {
		CPPUNIT_ASSERT_MESSAGE(
			"chunk linked with non-neighbor",
			!chunk->HasNeighbor(Block::Face(i))
		);
	}
}


void ChunkTest::testBlock() {
	unique_ptr<Chunk> chunk(new Chunk(types));

	for (int index = 0; index < Chunk::size; ++index) {
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"default chunk has non-default block",
			Block(), chunk->BlockAt(index)
		);
	}

	Block block(1, Block::FACE_LEFT, Block::TURN_RIGHT);
	chunk->SetBlock(0, block);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong type on set block",
		block.type, chunk->BlockAt(0).type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong orientation on set block",
		block.orient, chunk->BlockAt(0).orient
	);
	for (int index = 1; index < Chunk::size; ++index) {
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"changing block at index 0 affected some other block",
			Block(), chunk->BlockAt(index)
		);
	}
}

void ChunkTest::testLight() {
	unique_ptr<Chunk> chunk(new Chunk(types));

	for (int index = 0; index < Chunk::size; ++index) {
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"default chunk has non-zero light level",
			0, chunk->GetLight(index)
		);
	}

	chunk->SetLight(0, 15);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong light level on set index",
		15, chunk->GetLight(0)
	);
	for (int index = 1; index < Chunk::size; ++index) {
		CPPUNIT_ASSERT_EQUAL_MESSAGE(
			"changing light at index 0 affected some other index",
			0, chunk->GetLight(index)
		);
	}
}

void ChunkTest::testLightPropagation() {
	unique_ptr<Chunk> chunk(new Chunk(types));

	// 0 air, 1 solid, 2 solid and emits light level of 5
	chunk->SetBlock(Chunk::Pos(7, 7, 7), Block(2));
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding luminant block did not set correct light level",
		5, chunk->GetLight(Chunk::Pos(7, 7, 7))
	);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"light did not propagate correctly in +X",
		4, chunk->GetLight(Chunk::Pos(8, 7, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"light did not propagate correctly in -X",
		4, chunk->GetLight(Chunk::Pos(6, 7, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"light did not propagate correctly in +Y",
		4, chunk->GetLight(Chunk::Pos(7, 8, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"light did not propagate correctly in -Y",
		4, chunk->GetLight(Chunk::Pos(7, 6, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"light did not propagate correctly in +Z",
		4, chunk->GetLight(Chunk::Pos(7, 7, 8))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"light did not propagate correctly in -Z",
		4, chunk->GetLight(Chunk::Pos(7, 7, 6))
	);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"light did not propagate correctly in 2D diagonal",
		3, chunk->GetLight(Chunk::Pos(8, 8, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"light did not propagate correctly in 2D diagonal",
		3, chunk->GetLight(Chunk::Pos(7, 6, 8))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"light did not propagate correctly in 2D diagonal",
		3, chunk->GetLight(Chunk::Pos(6, 7, 8))
	);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"light did not propagate correctly in 3D diagonal",
		2, chunk->GetLight(Chunk::Pos(8, 6, 6))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"light did not propagate correctly in 3D diagonal",
		2, chunk->GetLight(Chunk::Pos(6, 6, 8))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"light did not propagate correctly in 3D diagonal",
		2, chunk->GetLight(Chunk::Pos(6, 8, 8))
	);

	// now block the light to the left
	chunk->SetBlock(Chunk::Pos(6, 7, 7), Block(1));
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		5, chunk->GetLight(Chunk::Pos(7, 7, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		4, chunk->GetLight(Chunk::Pos(8, 7, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		4, chunk->GetLight(Chunk::Pos(7, 8, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		4, chunk->GetLight(Chunk::Pos(7, 6, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		4, chunk->GetLight(Chunk::Pos(7, 7, 8))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		4, chunk->GetLight(Chunk::Pos(7, 7, 6))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		3, chunk->GetLight(Chunk::Pos(6, 6, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		3, chunk->GetLight(Chunk::Pos(6, 8, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		3, chunk->GetLight(Chunk::Pos(6, 7, 6))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		3, chunk->GetLight(Chunk::Pos(6, 7, 6))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		2, chunk->GetLight(Chunk::Pos(5, 6, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		2, chunk->GetLight(Chunk::Pos(5, 8, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		2, chunk->GetLight(Chunk::Pos(5, 7, 6))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle affected unrelated index",
		2, chunk->GetLight(Chunk::Pos(5, 7, 6))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"adding obstacle resulted in unexpected light level behind it",
		1, chunk->GetLight(Chunk::Pos(5, 7, 7))
	);

	// and remove it again
	chunk->SetBlock(Chunk::Pos(6, 7, 7), Block(0));
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"removing obstacle did not refill light correctly",
		4, chunk->GetLight(Chunk::Pos(6, 7, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"removing obstacle did not refill light correctly",
		3, chunk->GetLight(Chunk::Pos(5, 7, 7))
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"removing obstacle did not refill light correctly",
		2, chunk->GetLight(Chunk::Pos(4, 7, 7))
	);
}

}
}
