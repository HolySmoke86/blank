#include "tcp.hpp"

#include "../app/init.hpp"

#include <stdexcept>

using namespace std;


namespace blank {
namespace tcp {

Socket::Socket()
: sock(nullptr) {

}

Socket::Socket(unsigned short port)
: sock(nullptr) {
	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, nullptr, port) == -1) {
		throw NetError("failed to resolve local host");
	}
	sock = SDLNet_TCP_Open(&ip);
	if (!sock) {
		throw NetError("failed to open local socket");
	}
}

Socket::Socket(const string &host, unsigned short port)
: sock(nullptr) {
	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, host.c_str(), port) == -1) {
		throw NetError("failed to resolve host " + host);
	}
	sock = SDLNet_TCP_Open(&ip);
	if (!sock) {
		throw NetError("failed to connect to " + host + ':' + to_string(port));
	}
}

Socket::Socket(TCPsocket sock)
: sock(sock) {

}

Socket::~Socket() noexcept {
	if (sock) {
		SDLNet_TCP_Close(sock);
	}
}

Socket::Socket(Socket &&other) noexcept
: sock(other.sock) {
	other.sock = nullptr;
}

Socket &Socket::operator =(Socket &&other) noexcept {
	swap(sock, other.sock);
	return *this;
}


Socket Socket::Accept() noexcept {
	return Socket(SDLNet_TCP_Accept(sock));
}

bool Socket::Ready() const noexcept {
	return SDLNet_SocketReady(sock);
}

size_t Socket::Recv(void *buf, std::size_t max_len) {
	const int len = SDLNet_TCP_Recv(sock, buf, max_len);
	if (len < 0) {
		throw NetError("TCP socket recv");
	}
	return len;
}

size_t Socket::Send(const void *buf, size_t max_len) {
	/// TODO: make TCP send non-blocking
	const int len = SDLNet_TCP_Send(sock, buf, max_len);
	if (len < int(max_len)) {
		throw NetError("TCP socket send");
	}
	return len;
}


int Socket::AddTo(SDLNet_SocketSet set) {
	return SDLNet_TCP_AddSocket(set, sock);
}

int Socket::RemoveFrom(SDLNet_SocketSet set) {
	return SDLNet_TCP_DelSocket(set, sock);
}


Pool::Pool(int max_conn, size_t buf_siz)
: set(SDLNet_AllocSocketSet(max_conn))
, connections()
, use_conn(0)
, max_conn(max_conn)
, buf_siz(buf_siz) {
	if (!set) {
		throw runtime_error("failed to allocate socket set");
	}
}

Pool::~Pool() noexcept {
	SDLNet_FreeSocketSet(set);
}


void Pool::AddConnection(Socket sock, IOHandler *handler) {
	if (FreeSlots() == 0) {
		Resize(TotalSlots() * 2);
	}
	int num = sock.AddTo(set);
	if (num < 0) {
		throw NetError("failed to add socket to set");
	}
	use_conn = num;
	connections.emplace_back(move(sock), handler);
	handler->OnCreate(connections.back().first);
}

void Pool::Send() {
	for (auto i = connections.begin(); i != connections.end(); ++i) {
		if (i->second->Closed()) {
			continue;
		}

		try {
			i->second->OnSend(i->first);
		} catch (...) {
			i->second->OnError(i->first);
		}
	}
}

bool Pool::Check(unsigned long timeout) {
	// SDL_net considers checking an empty set an error, so
	// we're checking that ourselves
	if (OccupiedSlots() == 0) {
		return false;
	}

	int num = SDLNet_CheckSockets(set, timeout);
	if (num < 0) {
		throw NetError("error checking sockets");
	}
	return num > 0;
}

void Pool::Receive() {
	for (auto i = connections.begin(); i != connections.end(); ++i) {
		if (!i->first.Ready() || i->second->Closed()) {
			continue;
		}

		try {
			i->second->OnRecv(i->first);
		} catch (...) {
			i->second->OnError(i->first);
		}
	}
}

void Pool::Clean() {
	for (auto i = connections.begin(); i != connections.end();) {
		if (i->second->Closed()) {
			int num = i->first.RemoveFrom(set);
			if (num < 0) {
				throw NetError("failed to remove socket from set");
			}
			use_conn = num;
			i->second->OnRemove(i->first);
			i = connections.erase(i);
		} else {
			++i;
		}
	}
}


void Pool::Resize(int new_max) {
	if (new_max < max_conn) {
		return;
	}

	int new_size = max(new_max, max_conn * 2);
	SDLNet_SocketSet new_set(SDLNet_AllocSocketSet(new_size));
	if (!new_set) {
		throw NetError("failed to allocate socket set");
	}

	for (auto &conn : connections) {
		if (conn.first.AddTo(new_set) == -1) {
			NetError error("failed to migrate socket to new set");
			SDLNet_FreeSocketSet(new_set);
			throw error;
		}
	}

	SDLNet_FreeSocketSet(set);
	set = new_set;
	max_conn = new_size;
}

}
}
