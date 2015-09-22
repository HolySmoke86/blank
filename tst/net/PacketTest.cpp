#include "PacketTest.hpp"

#include "model/CompositeModel.hpp"
#include "world/Entity.hpp"

#include <limits>

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

void PacketTest::testControl() {
	Packet::TControl ctrl;
	ctrl.ack = 10;

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
	AssertPacket("Join", 2, 68, 100, pack);

	Entity write_entity;
	write_entity.ID(534574);
	write_entity.GetState().chunk_pos = { 7, 2, -3 };
	write_entity.GetState().block_pos = { 1.5f, 0.9f, 12.0f };
	write_entity.GetState().velocity = { 0.025f, 0.001f, 0.0f };
	write_entity.GetState().orient = { 1.0f, 0.0f, 0.0f, 0.0f };
	write_entity.GetState().ang_vel = { 0.01f, 0.00302f, 0.0985f };
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
	AssertPacket("PlayerUpdate", 4, 64, pack);

	Entity write_entity;
	write_entity.ID(534574);
	write_entity.GetState().chunk_pos = { 7, 2, -3 };
	write_entity.GetState().block_pos = { 1.5f, 0.9f, 12.0f };
	write_entity.GetState().velocity = { 0.025f, 0.001f, 0.0f };
	write_entity.GetState().orient = { 1.0f, 0.0f, 0.0f, 0.0f };
	write_entity.GetState().ang_vel = { 0.01f, 0.00302f, 0.0985f };
	EntityState read_state;
	pack.WritePlayer(write_entity);

	pack.ReadPlayerState(read_state);
	AssertEqual(
		"player entity state not correctly transported in PlayerUpdate packet",
		write_entity.GetState(), read_state
	);
}

void PacketTest::testSpawnEntity() {
	auto pack = Packet::Make<Packet::SpawnEntity>(udp_pack);
	AssertPacket("SpawnEntity", 5, 100, 132, pack);

	Entity write_entity;
	write_entity.ID(534574);
	CompositeModel model;
	model.ID(23);
	model.Instantiate(write_entity.GetModel());
	write_entity.GetState().chunk_pos = { 7, 2, -3 };
	write_entity.GetState().block_pos = { 1.5f, 0.9f, 12.0f };
	write_entity.GetState().velocity = { 0.025f, 0.001f, 0.0f };
	write_entity.GetState().orient = { 1.0f, 0.0f, 0.0f, 0.0f };
	write_entity.GetState().ang_vel = { 0.01f, 0.00302f, 0.0985f };
	write_entity.Bounds({{ -1, -1, -1 }, { 1, 1, 1 }});
	write_entity.WorldCollidable(true);
	write_entity.Name("blah");
	pack.WriteEntity(write_entity);

	uint32_t entity_id;
	uint32_t skeleton_id;
	Entity read_entity;
	pack.ReadEntityID(entity_id);
	pack.ReadSkeletonID(skeleton_id);
	pack.ReadEntity(read_entity);

	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"entity ID not correctly transported in SpawnEntity packet",
		write_entity.ID(), entity_id
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"skeleton ID not correctly transported in SpawnEntity packet",
		write_entity.GetModel().GetModel().ID(), skeleton_id
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
	AssertPacket("EntityUpdate", 7, 4, 452, pack);

	pack.length = Packet::EntityUpdate::GetSize(3);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"length not correctly set in DespawnEntity packet",
		size_t(4 + 3 * 68), pack.length
	);

	uint32_t write_count = 3;
	uint32_t read_count;
	pack.WriteEntityCount(write_count);
	pack.ReadEntityCount(read_count);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"entity count not correctly transported in EntityUpdate packet",
		write_count, read_count
	);

	Entity write_entity;
	write_entity.ID(8567234);
	write_entity.GetState().chunk_pos = { 7, 2, -3 };
	write_entity.GetState().block_pos = { 1.5f, 0.9f, 12.0f };
	write_entity.GetState().velocity = { 0.025f, 0.001f, 0.0f };
	write_entity.GetState().orient = { 1.0f, 0.0f, 0.0f, 0.0f };
	write_entity.GetState().ang_vel = { 0.01f, 0.00302f, 0.0985f };
	pack.WriteEntity(write_entity, 1);
	pack.WriteEntity(write_entity, 0);
	pack.WriteEntity(write_entity, 2);

	uint32_t read_id;
	EntityState read_state;
	pack.ReadEntityID(read_id, 1);
	pack.ReadEntityState(read_state, 1);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"entity ID not correctly transported in EntityUpdate packet",
		write_entity.ID(), read_id
	);
	AssertEqual(
		"entity state not correctly transported in EntityUpdate packet",
		write_entity.GetState(), read_state
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
		expected.block_pos, actual.block_pos
	);
	AssertEqual(
		message + ": bad velocity",
		expected.velocity, actual.velocity
	);
	AssertEqual(
		message + ": bad orientation",
		expected.orient, actual.orient
	);
	AssertEqual(
		message + ": bad angular velocity",
		expected.ang_vel, actual.ang_vel
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
	const glm::vec3 &actual
) {
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
