#include "WorldSave.hpp"

#include "filesystem.hpp"
#include "TokenStreamReader.hpp"

#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <zlib.h>
#include <glm/gtx/io.hpp>

using namespace std;


namespace blank {

WorldSave::WorldSave(const string &path)
: root_path(path)
, world_conf_path(path + "world.conf")
, gen_conf_path(path + "gen.conf")
, player_path(path + "player/")
, chunk_path(path + "chunks/%d/%d/%d.gz")
, chunk_bufsiz(chunk_path.length() + 3 * std::numeric_limits<int>::digits10)
, chunk_buf(new char[chunk_bufsiz]) {

}


bool WorldSave::Exists() const noexcept {
	return is_dir(root_path) && is_file(world_conf_path);
}


void WorldSave::Read(World::Config &conf) const {
	ifstream is(world_conf_path);
	if (!is) {
		throw runtime_error("failed to open world config");
	}
	TokenStreamReader in(is);

	string name;
	while (in.HasMore()) {
		in.ReadIdentifier(name);
		in.Skip(Token::EQUALS);
		if (name == "spawn") {
			in.ReadVec(conf.spawn);
		}
		if (in.HasMore() && in.Peek().type == Token::SEMICOLON) {
			in.Skip(Token::SEMICOLON);
		}
	}

	if (is.bad()) {
		throw runtime_error("IO error reading world config");
	}
}

void WorldSave::Write(const World::Config &conf) const {
	if (!make_dirs(root_path)) {
		throw runtime_error("failed to create world save directory");
	}

	ofstream out(world_conf_path);
	out << "spawn = " << conf.spawn << ';' << endl;
	out.close();

	if (!out) {
		throw runtime_error("failed to write world config");
	}
}


void WorldSave::Read(Generator::Config &conf) const {
	ifstream is(gen_conf_path);
	if (!is) {
		throw runtime_error("failed to open generator config");
	}
	TokenStreamReader in(is);

	string name;
	while (in.HasMore()) {
		in.ReadIdentifier(name);
		in.Skip(Token::EQUALS);
		if (name == "seed") {
			in.ReadNumber(conf.seed);
		}
		if (in.HasMore() && in.Peek().type == Token::SEMICOLON) {
			in.Skip(Token::SEMICOLON);
		}
	}

	if (is.bad()) {
		throw runtime_error("IO error reading generator config");
	}
}

void WorldSave::Write(const Generator::Config &conf) const {
	if (!make_dirs(root_path)) {
		throw runtime_error("failed to create world save directory");
	}

	ofstream out(gen_conf_path);
	out << "seed = " << conf.seed << ';' << endl;
	out.close();

	if (!out) {
		throw runtime_error("failed to write generator config");
	}
}


bool WorldSave::Exists(const Player &player) const {
	return is_file(PlayerPath(player));
}

void WorldSave::Read(Player &player) const {
	ifstream is(PlayerPath(player));
	TokenStreamReader in(is);
	string name;
	EntityState state;
	while (in.HasMore()) {
		in.ReadIdentifier(name);
		in.Skip(Token::EQUALS);
		if (name == "chunk") {
			in.ReadVec(state.chunk_pos);
		} else if (name == "position") {
			in.ReadVec(state.block_pos);
		} else if (name == "orientation") {
			in.ReadQuat(state.orient);
		} else if (name == "pitch") {
			state.pitch = in.GetFloat();
		} else if (name == "yaw") {
			state.yaw = in.GetFloat();
		} else if (name == "slot") {
			int slot;
			in.ReadNumber(slot);
			player.SetInventorySlot(slot);
		}
		if (in.HasMore() && in.Peek().type == Token::SEMICOLON) {
			in.Skip(Token::SEMICOLON);
		}
	}
	player.GetEntity().SetState(state);
}

void WorldSave::Write(const Player &player) const {
	if (!make_dirs(player_path)) {
		throw runtime_error("failed to create player save directory");
	}
	const EntityState &state = player.GetEntity().GetState();
	ofstream out(PlayerPath(player));
	out << "chunk = " << state.chunk_pos << ';' << endl;
	out << "position = " << state.block_pos << ';' << endl;
	out << "orientation = " << state.orient << ';' << endl;
	out << "pitch = " << state.pitch << ';' << endl;
	out << "yaw = " << state.yaw << ';' << endl;
	out << "slot = " << player.GetInventorySlot() << ';' << endl;
}

string WorldSave::PlayerPath(const Player &player) const {
	// TODO: this is potentially dangerous, server and client should
	//       provide a sanitized name for storage
	return player_path + player.Name();
}


bool WorldSave::Exists(const Chunk::Pos &pos) const noexcept {
	return is_file(ChunkPath(pos));
}

void WorldSave::Read(Chunk &chunk) const {
	const char *path = ChunkPath(chunk.Position());
	gzFile file = gzopen(path, "r");
	if (!file) {
		throw runtime_error("failed to open chunk file");
	}
	if (gzread(file, chunk.BlockData(), Chunk::BlockSize()) != Chunk::BlockSize()) {
		throw runtime_error("failed to read chunk from file");
	}
	if (gzclose(file) != Z_OK) {
		throw runtime_error("failed to read chunk file");
	}
	chunk.InvalidateMesh();
	chunk.ClearSave();
}

void WorldSave::Write(Chunk &chunk) const {
	const char *path = ChunkPath(chunk.Position());
	gzFile file = gzopen(path, "w");
	if (!file) {
		// check if it's because of a missing path component
		if (errno != ENOENT) {
			// nope, fatal
			throw runtime_error(strerror(errno));
		}
		string dir_path(path);
		dir_path.erase(dir_path.find_last_of("\\/"));
		if (!make_dirs(dir_path)) {
			throw runtime_error("failed to create dir for chunk file");
		}
		file = gzopen(path, "w");
		if (!file) {
			throw runtime_error("failed to open chunk file");
		}
	}
	if (gzwrite(file, chunk.BlockData(), Chunk::BlockSize()) == 0) {
		gzclose(file); // if this fails, it can't be helped
		throw runtime_error("failed to write chunk to file");
	}
	if (gzclose(file) != Z_OK) {
		throw runtime_error("failed to write chunk file");
	}
	chunk.ClearSave();
}


const char *WorldSave::ChunkPath(const Chunk::Pos &pos) const {
	snprintf(chunk_buf.get(), chunk_bufsiz, chunk_path.c_str(), pos.x, pos.y, pos.z);
	return chunk_buf.get();
}

}
