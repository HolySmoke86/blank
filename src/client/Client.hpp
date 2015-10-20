#ifndef BLANK_CLIENT_CLIENT_HPP_
#define BLANK_CLIENT_CLIENT_HPP_

#include "../app/Config.hpp"
#include "../net/Connection.hpp"

#include <string>
#include <SDL_net.h>


namespace blank {

class World;

namespace client {

class Client {

public:
	explicit Client(const Config::Network &);
	~Client();

	void Handle();

	void Update(int dt);

	Connection &GetConnection() noexcept { return conn; }
	const Connection &GetConnection() const noexcept { return conn; }

	std::uint16_t SendPing();
	std::uint16_t SendLogin(const std::string &);
	std::uint16_t SendPart();
	std::uint16_t SendPlayerUpdate(
		const EntityState &prediction,
		const glm::vec3 &movement,
		float pitch,
		float yaw,
		std::uint8_t actions,
		std::uint8_t slot);
	std::uint16_t SendMessage(
		std::uint8_t type,
		std::uint32_t ref,
		const std::string &msg);

private:
	void HandlePacket(const UDPpacket &);

private:
	Connection conn;
	UDPsocket client_sock;
	UDPpacket client_pack;

};

}
}

#endif
