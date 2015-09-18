#ifndef BLANK_CLIENT_CHUNKTRANSMISSION_HPP_
#define BLANK_CLIENT_CHUNKTRANSMISSION_HPP_

#include "../world/Chunk.hpp"

#include <cstdint>
#include <glm/glm.hpp>


namespace blank {
namespace client {

struct ChunkTransmission {

	std::uint32_t id;
	std::uint32_t flags;
	glm::ivec3 coords;
	std::uint32_t data_size;
	std::uint32_t data_received;

	int last_update;

	bool header_received;
	bool active;

	std::uint8_t buffer[Chunk::BlockSize() + 10];


	ChunkTransmission();

	void Clear() noexcept;

	bool Complete() const noexcept;

	bool Compressed() const noexcept;

};

}
}

#endif
