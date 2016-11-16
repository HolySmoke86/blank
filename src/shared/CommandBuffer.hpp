#ifndef BLANK_SHARED_COMMANDBUFFER_HPP_
#define BLANK_SHARED_COMMANDBUFFER_HPP_

#include "CLIContext.hpp"
#include "../net/tcp.hpp"

#include <string>


namespace blank {

/// Turns a tcp stream into commands and writes their
/// output back to the stream.
/// Instances delete themselves when OnRemove(tcp::Socket &)
/// is called, so make sure it was either allocated with new
/// and isn't dereferenced after removal or OnRemove is never
/// called.
class CommandBuffer
: public CLIContext
, public tcp::IOHandler {

public:
	explicit CommandBuffer(CLI &);
	~CommandBuffer() override;

	// CLIContext implementation
	void Error(const std::string &) override;
	void Message(const std::string &) override;
	void Broadcast(const std::string &) override;

	/// IOHandler implementation
	void OnSend(tcp::Socket &) override;
	void OnRecv(tcp::Socket &) override;
	void OnRemove(tcp::Socket &) noexcept override;

private:
	CLI &cli;
	std::string write_buffer;
	std::string read_buffer;
	std::size_t head;

};

}

#endif
