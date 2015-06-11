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
		chunk->Unlink();
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

}
}
