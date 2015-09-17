#include "PacketTest.hpp"

#include "net/Packet.hpp"
#include "world/Entity.hpp"
#include "world/EntityState.hpp"

#include <string>

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
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"Ping packet not correctly tagged",
		TEST_TAG, pack.GetHeader().tag
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong type code for Ping packet",
		uint8_t(0), pack.GetHeader().type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad payload length for Ping packet",
		size_t(0), pack.length
	);
}

void PacketTest::testLogin() {
	auto pack = Packet::Make<Packet::Login>(udp_pack);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"Login packet not correctly tagged",
		TEST_TAG, pack.GetHeader().tag
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong type code for Login packet",
		uint8_t(1), pack.GetHeader().type
	);
	CPPUNIT_ASSERT_MESSAGE(
		"bad payload length for Login packet",
		pack.length <= 32
	);

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
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"Join packet not correctly tagged",
		TEST_TAG, pack.GetHeader().tag
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong type code for Join packet",
		uint8_t(2), pack.GetHeader().type
	);
	CPPUNIT_ASSERT_MESSAGE(
		"bad payload length for Join packet",
		pack.length >= 68 && pack.length <= 100
	);

	Entity write_entity;
	write_entity.ID(534574);
	uint32_t read_id = 0;
	pack.WritePlayer(write_entity);

	pack.ReadPlayerID(read_id);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"player entity ID not correctly transported in Join packet",
		write_entity.ID(), read_id
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
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"Part packet not correctly tagged",
		TEST_TAG, pack.GetHeader().tag
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"wrong type code for Part packet",
		uint8_t(3), pack.GetHeader().type
	);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"bad payload length for Part packet",
		size_t(0), pack.length
	);
}

}
}
