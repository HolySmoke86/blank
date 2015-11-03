#ifndef BLANK_IO_WORLDSAVE_HPP_
#define BLANK_IO_WORLDSAVE_HPP_

#include "../world/Chunk.hpp"
#include "../world/Generator.hpp"
#include "../world/World.hpp"

#include <memory>
#include <string>


namespace blank {

class Player;

class WorldSave {

public:
	explicit WorldSave(const std::string &path);

public:
	// whole save
	bool Exists() const noexcept;
	void Read(World::Config &) const;
	void Write(const World::Config &) const;
	void Read(Generator::Config &) const;
	void Write(const Generator::Config &) const;

	// player
	bool Exists(const Player &) const;
	void Read(Player &) const;
	void Write(const Player &) const;
	std::string PlayerPath(const Player &) const;

	// single chunk
	bool Exists(const ExactLocation::Coarse &) const noexcept;
	void Read(Chunk &) const;
	void Write(Chunk &) const;
	const char *ChunkPath(const ExactLocation::Coarse &) const;

private:
	std::string root_path;
	std::string world_conf_path;
	std::string gen_conf_path;
	std::string player_path;
	std::string chunk_path;
	std::size_t chunk_bufsiz;
	std::unique_ptr<char[]> chunk_buf;

};

}

#endif
