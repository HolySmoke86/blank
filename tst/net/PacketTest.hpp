#ifndef BLANK_TEST_NET_PACKETTEST_HPP_
#define BLANK_TEST_NET_PACKETTEST_HPP_

#include <SDL_net.h>
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

CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testControl();
	void testPing();
	void testLogin();
	void testJoin();
	void testPart();

private:
	UDPpacket udp_pack;

};

}
}

#endif
