#ifndef BLANK_SERVER_CHUNKTRANSMITTER_HPP_
#define BLANK_SERVER_CHUNKTRANSMITTER_HPP_

#include <cstdint>
#include <memory>
#include <vector>


namespace blank {

class Chunk;

namespace server {

class ClientConnection;

class ChunkTransmitter {

public:
	explicit ChunkTransmitter(ClientConnection &);
	~ChunkTransmitter();

	/// Returns true if not transmitting or waiting on acks, so
	/// the next chunk may be queued without schmutzing up anything.
	bool Idle() const noexcept;

	/// Returns true if a transmission is still going on,
	/// meaning there's at least one packet that needs to
	/// be sent.
	bool Transmitting() const noexcept;
	/// Send the next packet of the current chunk (if any).
	void Transmit();

	/// Returns true if there's one or more packets which
	/// still have to be ack'd by the remote.
	bool Waiting() const noexcept;
	/// Mark packet with given sequence number as ack'd.
	/// If all packets for the current chunk have been ack'd
	/// the transmission is considered complete.
	void Ack(std::uint16_t);
	/// Mark packet with given sequence number as lost.
	/// Its part of the chunk data should be resent.
	void Nack(std::uint16_t);

	/// Cancel the current transmission.
	void Abort();
	/// Start transmitting given chunk.
	/// If there's a chunk already in transmission it will be
	/// cancelled.
	void Send(Chunk &);

private:
	void SendBegin();
	void SendData(std::size_t);
	void Release();

private:
	ClientConnection &conn;
	Chunk *current;
	std::size_t buffer_size;
	std::unique_ptr<std::uint8_t[]> buffer;
	std::size_t buffer_len;
	std::size_t packet_len;
	std::size_t cursor;
	std::size_t num_packets;
	int begin_packet;
	std::vector<int> data_packets;
	int confirm_wait;
	std::uint32_t trans_id;
	bool compressed;

};

}
}

#endif
