#ifndef BLANK_IO_WORLDSAVE_HPP_
#define BLANK_IO_WORLDSAVE_HPP_

#include "../world/Chunk.hpp"
#include "../world/World.hpp"

#include <memory>
#include <string>


namespace blank {

class WorldSave {

public:
	explicit WorldSave(const std::string &path);

public:
	// whole save
	bool Exists() const noexcept;
	void Read(World::Config &) const;
	void Write(const World::Config &) const;

	// single chunk
	bool Exists(const Chunk::Pos &) const noexcept;
	void Read(Chunk &) const;
	void Write(Chunk &) const;

	const char *ChunkPath(const Chunk::Pos &) const;

private:
	std::string root_path;
	std::string conf_path;
	std::string chunk_path;
	std::size_t chunk_bufsiz;
	std::unique_ptr<char[]> chunk_buf;

};

}

#endif
