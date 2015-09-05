#ifndef BLANK_NET_CLIENTCONNECTION_HPP_
#define BLANK_NET_CLIENTCONNECTION_HPP_

#include "Connection.hpp"
#include "ConnectionHandler.hpp"

#include <SDL_net.h>


namespace blank {

class Entity;
class Server;

class ClientConnection
: public ConnectionHandler {

public:
	explicit ClientConnection(Server &, const IPaddress &);
	~ClientConnection();

	bool Matches(const IPaddress &addr) const noexcept { return conn.Matches(addr); }

	void Update(int dt);

	Connection &GetConnection() noexcept { return conn; }
	bool Disconnected() const noexcept { return conn.Closed(); }

	void AttachPlayer(Entity &);
	void DetachPlayer();

	void On(const Packet::Login &) override;
	void On(const Packet::Part &) override;

private:
	Server &server;
	Connection conn;
	Entity *player;

};

}

#endif
