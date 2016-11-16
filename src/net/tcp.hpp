#ifndef BLANK_NET_TCP_HPP_
#define BLANK_NET_TCP_HPP_

#include <algorithm>
#include <list>
#include <SDL_net.h>
#include <string>


namespace blank {
namespace tcp {

/// all failing functions throw NetError
class Socket {

public:
	/// create an empty socket that is not connected to anything
	Socket();
	/// create TCP socket bound to given port
	explicit Socket(unsigned short port);
private:
	/// wrap given SDLNet TCP socket
	/// for use with Accept()
	explicit Socket(TCPsocket sock);
public:
	~Socket() noexcept;

	Socket(const Socket &) = delete;
	Socket &operator =(const Socket &) = delete;

	Socket(Socket &&) noexcept;
	Socket &operator =(Socket &&) noexcept;

	explicit operator bool() const noexcept { return sock; }

	bool operator ==(const Socket &other) const noexcept {
		return sock == other.sock;
	}
	bool operator <(const Socket &other) const noexcept {
		return sock < other.sock;
	}

public:
	/// create a socket for an incoming connection
	/// @return an empty socket if there are none
	Socket Accept() noexcept;
	/// check if there is data available to read
	bool Ready() const noexcept;
	/// receive data into given buffer
	/// @return number of bytes read, at most max_len
	/// non-blocking if Ready() is true
	std::size_t Recv(void *buf, std::size_t max_len);
	/// send data from given buffer, at most max_len bytes
	/// @return number of bytes written
	///         may be less than len as soon as I get to
	///         making it non-blocking
	std::size_t Send(const void *buf, std::size_t max_len);

	int AddTo(SDLNet_SocketSet);
	int RemoveFrom(SDLNet_SocketSet);

private:
	TCPsocket sock;

};


struct IOHandler {

	virtual ~IOHandler() = default;

	void Close() noexcept { closed = true; }
	bool Closed() const noexcept { return closed; }

	virtual void OnCreate(Socket &) { }
	virtual void OnRemove(Socket &) noexcept { }

	virtual void OnSend(Socket &) { };
	virtual void OnRecv(Socket &) { };
	virtual void OnError(Socket &) noexcept { Close(); }

private:
	bool closed = false;

};


class Pool {

public:
	using ConnectionSet = std::list<std::pair<Socket, IOHandler *>>;

public:
	explicit Pool(int max_conn = 32, std::size_t buf_siz = 1500);
	~Pool() noexcept;

	Pool(const Pool &) = delete;
	Pool &operator =(const Pool &) = delete;

public:
	void AddConnection(Socket, IOHandler *);
	void Send();
	bool Check(unsigned long timeout);
	void Receive();
	void Clean();

	int FreeSlots() const noexcept { return max_conn - use_conn; }
	int OccupiedSlots() const noexcept { return use_conn; }
	int TotalSlots() const noexcept { return max_conn; }
	/// reallocate the pool to accomodate at least new_max sockets
	void Resize(int new_max);

private:
	SDLNet_SocketSet set;
	std::string buffer;
	ConnectionSet connections;
	int use_conn;
	int max_conn;
	std::size_t buf_siz;

};

}
}

#endif
