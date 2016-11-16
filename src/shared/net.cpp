#include "CommandService.hpp"

#include "CLI.hpp"
#include "CommandBuffer.hpp"

#include <algorithm>
#include <iostream>

using namespace std;


namespace blank {

CommandService::CommandService(CLI &cli, unsigned short port)
: cli(cli)
, pool() {
	pool.AddConnection(tcp::Socket(port), this);
	cout << "listening on TCP port " << port << endl;
}

CommandService::~CommandService() {

}


void CommandService::Wait(int timeout) noexcept {
	pool.Check(timeout);
}

bool CommandService::Ready() noexcept {
	return pool.Check(0);
}

void CommandService::Handle() {
	pool.Receive();
}

void CommandService::Send() {
	pool.Send();
}

void CommandService::OnRecv(tcp::Socket &serv) {
	for (tcp::Socket client = serv.Accept(); client; client = serv.Accept()) {
		pool.AddConnection(move(client), new CommandBuffer(cli));
	}
}


CommandBuffer::CommandBuffer(CLI &cli)
: cli(cli)
, write_buffer()
, read_buffer(1440, '\0')
, head(0) {

}

CommandBuffer::~CommandBuffer() {

}


void CommandBuffer::Error(const string &msg) {
	write_buffer += " ! ";
	write_buffer += msg;
	write_buffer += '\n';
}

void CommandBuffer::Message(const string &msg) {
	write_buffer += " > ";
	write_buffer += msg;
	write_buffer += '\n';
}

void CommandBuffer::Broadcast(const string &msg) {
	// TODO: broadcast should be an operation of the
	// environment, not the singular context
	write_buffer += " @ ";
	write_buffer += msg;
	write_buffer += '\n';
}


void CommandBuffer::OnSend(tcp::Socket &sock) {
	if (write_buffer.empty()) {
		return;
	}
	size_t len = sock.Send(write_buffer.data(), write_buffer.size());
	write_buffer.erase(0, len);
}

void CommandBuffer::OnRecv(tcp::Socket &sock) {
	size_t len = sock.Recv(&read_buffer[0], read_buffer.size() - head);
	head += len;
	// scan for lines
	string::iterator begin = read_buffer.begin();
	string::iterator end = begin + head;
	string::iterator handled = begin;
	for (
		string::iterator i = find(handled, end, '\n');
		i != end;
		i = find(handled, end, '\n')
	) {
		string line(handled, i);
		cli.Execute(*this, line);
		handled = ++i;
	}
	if (handled == end) {
		// guzzled it all
		head = 0;
	} else if (handled != begin) {
		// half a line remaining, move it to the start of the buffer
		move(handled, end, begin);
		head = distance(handled, end);
	}
}

void CommandBuffer::OnRemove(tcp::Socket &) noexcept {
	delete this;
}

}
