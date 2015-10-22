#include "PacketTest.hpp"

#include "model/Model.hpp"
#include "world/Entity.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::PacketTest);

using namespace std;

namespace blank {
namespace test {

void PacketTest::setUp() {
	udp_pack.data = new Uint8[sizeof(Packet)];
	udp_pack.maxlen = sizeof(Packet);
}

void PacketTest::tearDown() {
	delete[] udp_pack.data;
}

namespace {

static constexpr uint32_t TEST_TAG = 0xFB1AB1AF;

}

void PacketTest::testSizes() {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected size of vec3",
		size_t(12), sizeof(glm::vec3)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"unexpected size of vec3i",
		size_t(12), sizeof(glm::ivec3)
	);
}

void PacketTest::testControl() {
	Packet::TControl ctrl{ 0, 10, 0 };

	CPPUNIT_ASSERT_MESSAGE(
		"TControl should ack the packet in the ack field",
		ctrl.Acks(10)
	);
	CPPUNIT_ASSERT_MESSAGE(
		"TControl should ack the packet in the future",
		!ctrl.Acks(11)
	);
	CPPUNIT_ASSERT_MESSAGE(
		"TControl should not ack a packet in the distant past",
		!ctrl.Acks(-30)
	);
	CPPUNIT_ASSERT_MESSAGE(
		"TControl should not ack the previous packet if the bitfield is 0",
		!ctrl.Acks(9)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"TControl's acks should begin at the packet in the ack field",
		uint16_t(10), ctrl.AckBegin()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"TControl's acks should end 33 packets before the one in the ack field",
		uint16_t(-23), ctrl.AckEnd()
	);
	ctrl.hist = 1;
	CPPUNIT_ASSERT_MESSAGE(
		"TControl should ack the previous packet if the bitfield is 1",
		ctrl.Acks(9)
	);
	ctrl.hist = 2;
	CPPUNIT_ASSERT_MESSAGE(
		"TControl should not ack the previous packet if the bitfield is 2",
		!ctrl.Acks(9)
	);
	CPPUNIT_ASSERT_MESSAGE(
		"TControl should ack the packet before the previous one if the bitfield is 2",
		ctrl.Acks(8)
	);
}

void PacketTest::testPing() {
	auto pack = Packet::Make<Packet::Ping>(udp_pack);
	AssertPacket("Ping", 0, 0, pack);
}

void PacketTest::testLogin() {
	auto pack = Packet::Make<Packet::Login>(udp_pack);
	AssertPacket("Login", 1, 0, 32, pack);

	string write_name = "test";
	string read_name;
	pack.WritePlayerName(write_name);
	pack.ReadPlayerName(read_name);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"player name not correctly transported in Login packet",
		write_name, read_name
	);

	write_name = "0123456789012345678901234567890123456789";
	pack.WritePlayerName(write_name);
	pack.ReadPlayerName(read_name);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"player name not correctly truncated in Login packet",
		write_name.substr(0, 32), read_name
	);
}

void PacketTest::testJoin() {
	auto pack = Packet::Make<Packet::Join>(udp_pack);
	AssertPacket("Join", 2, 47, 78, pack);

	Entity write_entity;
	write_entity.ID(534574);
	EntityState write_state;
	write_state.chunk_pos = { 7, 2, -3 };
	write_state.block_pos = { 1.5f, 0.9f, 12.0f };
	write_state.velocity = { 0.025f, 0.001f, 0.0f };
	write_state.orient = { 1.0f, 0.0f, 0.0f, 0.0f };
	write_state.pitch = 0.3f;
	write_state.yaw = -2.3f;
	write_entity.SetState(write_state);
	uint32_t read_id = 0;
	EntityState read_state;
	pack.WritePlayer(write_entity);

	pack.ReadPlayerID(read_id);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"player entity ID not correctly transported in Join packet",
		write_entity.ID(), read_id
	);
	pack.ReadPlayerState(read_state);
	AssertEqual(
		"player entity state not correctly transported in Join packet",
		write_entity.GetState(), read_state
	);

	string write_name = "test";
	string read_name;
	pack.WriteWorldName(write_name);
	pack.ReadWorldName(read_name);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"world name not correctly transported in Join packet",
		write_name, read_name
	);

	write_name = "0123456789012345678901234567890123456789";
	pack.WriteWorldName(write_name);
	pack.ReadWorldName(read_name);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"world name not correctly truncated in Join packet",
		write_name.substr(0, 32), read_name
	);
}

void PacketTest::testPart() {
	auto pack = Packet::Make<Packet::Part>(udp_pack);
	AssertPacket("Part", 3, 0, pack);
}

void PacketTest::testPlayerUpdate() {
	auto pack = Packet::Make<Packet::PlayerUpdate>(udp_pack);
	AssertPacket("PlayerUpdate", 4, 50, pack);

	EntityState write_state;
	write_state.chunk_pos = { 7, 2, -3 };
	write_state.block_pos = { 1.5f, 0.9f, 12.0f };
	write_state.velocity = { 0.025f, 0.001f, 0.0f };
	write_state.orient = { 1.0f, 0.0f, 0.0f, 0.0f };
	glm::vec3 write_movement(0.5f, -1.0f, 1.0f);
	uint8_t write_actions = 0x05;
	uint8_t write_slot = 3;
	pack.WritePredictedState(write_state);
	pack.WriteMovement(write_movement);
	pack.WriteActions(write_actions);
	pack.WriteSlot(write_slot);

	EntityState read_state;
	glm::vec3 read_movement;
	uint8_t read_actions;
	uint8_t read_slot;
	pack.ReadPredictedState(read_state);
	pack.ReadMovement(read_movement);
	pack.ReadActions(read_actions);
	pack.ReadSlot(read_slot);
	AssertEqual(
		"player predicted entity state not correctly transported in PlayerUpdate packet",
		write_state, read_state
	);
	AssertEqual(
		"player movement input not correctly transported in PlayerUpdate packet",
		write_movement, read_movement, 0.0001f
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"player actions not correctly transported in PlayerUpdate packet",
		int(write_actions), int(read_actions)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"player inventory slot not correctly transported in PlayerUpdate packet",
		int(write_slot), int(read_slot)
	);
}

void PacketTest::testSpawnEntity() {
	auto pack = Packet::Make<Packet::SpawnEntity>(udp_pack);
	AssertPacket("SpawnEntity", 5, 79, 110, pack);

	Entity write_entity;
	write_entity.ID(534574);
	Model model;
	model.ID(23);
	model.Enumerate();
	model.Instantiate(write_entity.GetModel());
	EntityState write_state;
	write_state.chunk_pos = { 7, 2, -3 };
	write_state.block_pos = { 1.5f, 0.9f, 12.0f };
	write_state.velocity = { 0.025f, 0.001f, 0.0f };
	write_state.orient = { 1.0f, 0.0f, 0.0f, 0.0f };
	write_state.pitch = 0.3f;
	write_state.yaw = -2.3f;
	write_entity.SetState(write_state);
	write_entity.Bounds({{ -1, -1, -1 }, { 1, 1, 1 }});
	write_entity.WorldCollidable(true);
	write_entity.Name("blah");
	pack.WriteEntity(write_entity);

	uint32_t entity_id;
	uint32_t model_id;
	Entity read_entity;
	pack.ReadEntityID(entity_id);
	pack.ReadModelID(model_id);
	pack.ReadEntity(read_entity);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"entity ID not correctly transported in SpawnEntity packet",
		write_entity.ID(), entity_id
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"model ID not correctly transported in SpawnEntity packet",
		write_entity.GetModel().GetModel().ID(), model_id
	);
	AssertEqual(
		"entity state not correctly transported in PlayerUpdate packet",
		write_entity.GetState(), read_entity.GetState()
	);
	AssertEqual(
		"entity bounds not correctly transported in PlayerUpdate packet",
		write_entity.Bounds(), read_entity.Bounds()
	);
	CPPUNIT_ASSERT_MESSAGE(
		"entity flags not correctly transported in SpawnEntity packet",
		read_entity.WorldCollidable()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"entity name not correctly transported in SpawnEntity packet",
		write_entity.Name(), read_entity.Name()
	);
}

void PacketTest::testDespawnEntity() {
	auto pack = Packet::Make<Packet::DespawnEntity>(udp_pack);
	AssertPacket("DespawnEntity", 6, 4, pack);

	uint32_t write_id = 5437;
	uint32_t read_id;
	pack.WriteEntityID(write_id);
	pack.ReadEntityID(read_id);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"entity ID not correctly transported in DespawnEntity packet",
		write_id, read_id
	);
}

void PacketTest::testEntityUpdate() {
	auto pack = Packet::Make<Packet::EntityUpdate>(udp_pack);
	AssertPacket("EntityUpdate", 7, 16, 460, pack);

	pack.length = Packet::EntityUpdate::GetSize(3);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"length not correctly set in EntityUpdate packet",
		size_t(16 + 3 * 37), pack.length
	);

	uint32_t write_count = 3;
	glm::ivec3 write_base(8, -15, 1);
	pack.WriteEntityCount(write_count);
	pack.WriteChunkBase(write_base);

	uint32_t read_count;
	glm::ivec3 read_base;
	pack.ReadEntityCount(read_count);
	pack.ReadChunkBase(read_base);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"entity count not correctly transported in EntityUpdate packet",
		write_count, read_count
	);
	AssertEqual(
		"chunk base not correctly transported in EntityUpdate packet",
		write_base, read_base
	);

	Entity write_entity;
	write_entity.ID(8567234);
	EntityState write_state;
	write_state.chunk_pos = { 7, 2, -3 };
	write_state.block_pos = { 1.5f, 0.9f, 12.0f };
	write_state.velocity = { 0.025f, 0.001f, 0.0f };
	write_state.orient = { 1.0f, 0.0f, 0.0f, 0.0f };
	write_state.pitch = 0.3f;
	write_state.yaw = -2.3f;
	write_entity.SetState(write_state);
	pack.WriteEntity(write_entity, write_base, 1);
	pack.WriteEntity(write_entity, write_base, 0);
	pack.WriteEntity(write_entity, write_base, 2);

	uint32_t read_id;
	EntityState read_state;
	pack.ReadEntityID(read_id, 1);
	pack.ReadEntityState(read_state, write_base, 1);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"entity ID not correctly transported in EntityUpdate packet",
		write_entity.ID(), read_id
	);
	AssertEqual(
		"entity state not correctly transported in EntityUpdate packet",
		write_entity.GetState(), read_state
	);
}

void PacketTest::testPlayerCorrection() {
	auto pack = Packet::Make<Packet::PlayerCorrection>(udp_pack);
	AssertPacket("PlayerCorrection", 8, 44, pack);

	uint16_t write_seq = 50050;
	uint16_t read_seq;
	pack.WritePacketSeq(write_seq);
	pack.ReadPacketSeq(read_seq);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"packet sequence not correctly transported in PlayerCorrection packet",
		write_seq, read_seq
	);

	Entity write_entity;
	EntityState write_state;
	write_state.chunk_pos = { 7, 2, -3 };
	write_state.block_pos = { 1.5f, 0.9f, 12.0f };
	write_state.velocity = { 0.025f, 0.001f, 0.0f };
	write_state.orient = { 1.0f, 0.0f, 0.0f, 0.0f };
	write_state.pitch = 0.3f;
	write_state.yaw = -2.3f;
	write_entity.SetState(write_state);
	pack.WritePlayer(write_entity);

	EntityState read_state;
	pack.ReadPlayerState(read_state);
	AssertEqual(
		"entity state not correctly transported in PlayerCorrection packet",
		write_entity.GetState(), read_state
	);
}

void PacketTest::testChunkBegin() {
	auto pack = Packet::Make<Packet::ChunkBegin>(udp_pack);
	AssertPacket("ChunkBegin", 9, 24, pack);

	uint32_t write_id = 532;
	uint32_t write_flags = 9864328;
	glm::ivec3 write_pos = { -6, 15, 38 };
	uint32_t write_size = 4097;

	pack.WriteTransmissionId(write_id);
	pack.WriteFlags(write_flags);
	pack.WriteChunkCoords(write_pos);
	pack.WriteDataSize(write_size);

	uint32_t read_id;
	uint32_t read_flags;
	glm::ivec3 read_pos;
	uint32_t read_size;

	pack.ReadTransmissionId(read_id);
	pack.ReadFlags(read_flags);
	pack.ReadChunkCoords(read_pos);
	pack.ReadDataSize(read_size);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"transmission ID not correctly transported in ChunkBegin packet",
		write_id, read_id
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"flags not correctly transported in ChunkBegin packet",
		write_flags, read_flags
	);
	AssertEqual(
		"chunk coordinates not correctly transported in ChunkBegin packet",
		write_pos, read_pos
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"data size not correctly transported in ChunkBegin packet",
		write_size, read_size
	);
}

void PacketTest::testChunkData() {
	auto pack = Packet::Make<Packet::ChunkData>(udp_pack);
	AssertPacket("ChunkData", 10, 12, 484, pack);

	constexpr size_t block_size = 97;

	uint32_t write_id = 6743124;
	uint32_t write_offset = 8583;
	uint32_t write_size = block_size;
	uint8_t write_data[block_size];
	memset(write_data, 'X', block_size);

	pack.WriteTransmissionId(write_id);
	pack.WriteDataOffset(write_offset);
	pack.WriteDataSize(write_size);
	pack.WriteData(write_data, write_size);

	uint32_t read_id;
	uint32_t read_offset;
	uint32_t read_size;
	uint8_t read_data[block_size];

	pack.ReadTransmissionId(read_id);
	pack.ReadDataOffset(read_offset);
	pack.ReadDataSize(read_size);
	pack.ReadData(read_data, read_size);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"transmission ID not correctly transported in ChunkData packet",
		write_id, read_id
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"data offset not correctly transported in ChunkData packet",
		write_offset, read_offset
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"data size not correctly transported in ChunkData packet",
		write_size, read_size
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"raw data not correctly transported in ChunkData packet",
		string(write_data, write_data + write_size), string(read_data, read_data + read_size)
	);
}

void PacketTest::testBlockUpdate() {
	auto pack = Packet::Make<Packet::BlockUpdate>(udp_pack);
	AssertPacket("BlockUpdate", 11, 16, 484, pack);

	pack.length = Packet::BlockUpdate::GetSize(3);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"length not correctly set in BlockUpdate packet",
		size_t(16 + 3 * 6), pack.length
	);

	glm::ivec3 write_coords(432, -325, 99998);
	uint32_t write_count = 3;
	uint16_t write_index = 432;
	Block write_block(324, Block::FACE_DOWN, Block::TURN_AROUND);

	pack.WriteChunkCoords(write_coords);
	pack.WriteBlockCount(write_count);
	pack.WriteIndex(write_index, 1);
	pack.WriteBlock(write_block, 1);
	pack.WriteIndex(write_index, 0);
	pack.WriteBlock(write_block, 0);
	pack.WriteIndex(write_index, 2);
	pack.WriteBlock(write_block, 2);

	glm::ivec3 read_coords;
	uint32_t read_count;
	uint16_t read_index;
	Block read_block;

	pack.ReadChunkCoords(read_coords);
	pack.ReadBlockCount(read_count);
	pack.ReadIndex(read_index, 1);
	pack.ReadBlock(read_block, 1);

	AssertEqual(
		"chunk coordinates not correctly transported in BlockUpdate packet",
		write_coords, read_coords
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"block count not correctly transported in BlockUpdate packet",
		write_count, read_count
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"block index not correctly transported in BlockUpdate packet",
		write_index, read_index
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"block type not correctly transported in BlockUpdate packet",
		write_block.type, read_block.type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"block face not correctly transported in BlockUpdate packet",
		write_block.GetFace(), read_block.GetFace()
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"block turn not correctly transported in BlockUpdate packet",
		write_block.GetTurn(), read_block.GetTurn()
	);
}

void PacketTest::testMessage() {
	auto pack = Packet::Make<Packet::Message>(udp_pack);
	AssertPacket("Message", 12, 6, 455, pack);

	const uint8_t write_type = 1;
	const uint32_t write_ref = 6433235;
	const string write_msg("hello, world");

	pack.length = Packet::Message::GetSize(write_msg);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"length not correctly set in BlockUpdate packet",
		size_t(5 + write_msg.size() + 1), pack.length
	);

	pack.WriteType(write_type);
	pack.WriteReferral(write_ref);
	pack.WriteMessage(write_msg);

	uint8_t read_type = 5;
	uint32_t read_ref = 884373;
	string read_msg;

	pack.ReadType(read_type);
	pack.ReadReferral(read_ref);
	pack.ReadMessage(read_msg);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"type not correctly transported in Message packet",
		write_type, read_type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"referral not correctly transported in Message packet",
		write_ref, read_ref
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"message not correctly transported in Message packet",
		write_msg, read_msg
	);
}


void PacketTest::AssertPacket(
	const string &name,
	uint8_t expected_type,
	size_t expected_length,
	const Packet::Payload &actual
) {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		name + " packet not correctly tagged",
		TEST_TAG, actual.GetHeader().tag
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong type code for " + name + " packet",
		int(expected_type), int(actual.GetHeader().type)
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad payload length for " + name + " packet",
		expected_length, actual.length
	);
}

void PacketTest::AssertPacket(
	const string &name,
	uint8_t expected_type,
	size_t min_length,
	size_t max_length,
	const Packet::Payload &actual
) {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		name + " packet not correctly tagged",
		TEST_TAG, actual.GetHeader().tag
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong type code for " + name + " packet",
		expected_type, actual.GetHeader().type
	);
	CPPUNIT_ASSERT_MESSAGE(
		"bad payload length for " + name + " packet",
		actual.length >= min_length && actual.length <= max_length
	);
}

void PacketTest::AssertEqual(
	const string &message,
	const EntityState &expected,
	const EntityState &actual
) {
	AssertEqual(
		message + ": bad chunk position",
		expected.chunk_pos, actual.chunk_pos
	);
	AssertEqual(
		message + ": bad block position",
		expected.block_pos, actual.block_pos, 16.0f/65535.0f // that's about the max accuracy that packing's going to give us
	);
	AssertEqual(
		message + ": bad velocity",
		expected.velocity, actual.velocity
	);
	AssertEqual(
		message + ": bad orientation",
		expected.orient, actual.orient
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		message + ": bad pitch",
		expected.pitch, actual.pitch, PI/65534.0f
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		message + ": bad yaw",
		expected.yaw, actual.yaw, PI/32767.0f
	);
}

void PacketTest::AssertEqual(
	const string &message,
	const AABB &expected,
	const AABB &actual
) {
	AssertEqual(
		message + ": bad lower bound",
		expected.min, actual.min
	);
	AssertEqual(
		message + ": bad upper bound",
		expected.max, actual.max
	);
}

void PacketTest::AssertEqual(
	const string &message,
	const glm::ivec3 &expected,
	const glm::ivec3 &actual
) {
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		message + " (X component)",
		expected.x, actual.x
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		message + " (Y component)",
		expected.y, actual.y
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		message + " (Z component)",
		expected.z, actual.z
	);
}

void PacketTest::AssertEqual(
	const string &message,
	const glm::vec3 &expected,
	const glm::vec3 &actual,
	float epsilon
) {
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		message + " (X component)",
		expected.x, actual.x, epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		message + " (Y component)",
		expected.y, actual.y, epsilon
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		message + " (Z component)",
		expected.z, actual.z, epsilon
	);
}

void PacketTest::AssertEqual(
	const string &message,
	const glm::quat &expected,
	const glm::quat &actual
) {
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		message + " (W component)",
		expected.w, actual.w, numeric_limits<float>::epsilon()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		message + " (X component)",
		expected.x, actual.x, numeric_limits<float>::epsilon()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		message + " (Y component)",
		expected.y, actual.y, numeric_limits<float>::epsilon()
	);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
		message + " (Z component)",
		expected.z, actual.z, numeric_limits<float>::epsilon()
	);
}

}
}
