#ifndef BLANK_TEST_NET_PACKETTEST_HPP_
#define BLANK_TEST_NET_PACKETTEST_HPP_

#include "model/geometry.hpp"
#include "net/Packet.hpp"
#include "world/EntityState.hpp"

#include <cstdint>
#include <string>
#include <SDL_net.h>
#include <glm/glm.hpp>
#include <cppunit/extensions/HelperMacros.h>


namespace blank {
namespace test {

class PacketTest
: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(PacketTest);

CPPUNIT_TEST(testControl);
CPPUNIT_TEST(testPing);
CPPUNIT_TEST(testLogin);
CPPUNIT_TEST(testJoin);
CPPUNIT_TEST(testPart);
CPPUNIT_TEST(testPlayerUpdate);
CPPUNIT_TEST(testSpawnEntity);
CPPUNIT_TEST(testDespawnEntity);
CPPUNIT_TEST(testEntityUpdate);

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testControl();
	void testPing();
	void testLogin();
	void testJoin();
	void testPart();
	void testPlayerUpdate();
	void testSpawnEntity();
	void testDespawnEntity();
	void testEntityUpdate();

private:
	static void AssertPacket(
		const std::string &name,
		std::uint8_t expected_type,
		std::size_t expected_length,
		const Packet::Payload &actual);
	static void AssertPacket(
		const std::string &name,
		std::uint8_t expected_type,
		std::size_t min_length,
		std::size_t max_length,
		const Packet::Payload &actual);

	static void AssertEqual(
		const std::string &message,
		const EntityState &expected,
		const EntityState &actual);
	static void AssertEqual(
		const std::string &message,
		const AABB &expected,
		const AABB &actual);
	static void AssertEqual(
		const std::string &message,
		const glm::ivec3 &expected,
		const glm::ivec3 &actual);
	static void AssertEqual(
		const std::string &message,
		const glm::vec3 &expected,
		const glm::vec3 &actual);
	static void AssertEqual(
		const std::string &message,
		const glm::quat &expected,
		const glm::quat &actual);

private:
	UDPpacket udp_pack;

};

}
}

#endif
