#ifndef BLANK_WORLD_WORLDSAVE_HPP_
#define BLANK_WORLD_WORLDSAVE_HPP_

#include "World.hpp"

#include <string>


namespace blank {

class WorldSave {

public:
	explicit WorldSave(const std::string &path);

public:
	bool Exists() const noexcept;

	void Create(const World::Config &) const;
	void Read(World::Config &) const;

private:
	std::string root_path;
	std::string conf_path;

};

}

#endif
