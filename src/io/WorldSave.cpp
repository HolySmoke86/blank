#include "WorldSave.hpp"

#include "filesystem.hpp"

#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <zlib.h>

using namespace std;


namespace blank {

WorldSave::WorldSave(const string &path)
: root_path(path)
, conf_path(path + "world.conf")
, chunk_path(path + "chunks/%d/%d/%d.gz")
, chunk_bufsiz(chunk_path.length() + 3 * std::numeric_limits<int>::digits10)
, chunk_buf(new char[chunk_bufsiz]) {

}


bool WorldSave::Exists() const noexcept {
	return is_dir(root_path) && is_file(conf_path);
}


void WorldSave::Read(World::Config &conf) const {
	ifstream in(conf_path);
	if (!in) {
		throw runtime_error("failed to open world config");
	}

	constexpr char spaces[] = "\n\r\t ";

	string line;
	while (getline(in, line)) {
		if (line.empty() || line[0] == '#') continue;
		auto equals_pos = line.find_first_of('=');

		auto name_begin = line.find_first_not_of(spaces, 0, sizeof(spaces));
		auto name_end = equals_pos - 1;
		while (name_end > name_begin && isspace(line[name_end])) {
			--name_end;
		}

		auto value_begin = line.find_first_not_of(spaces, equals_pos + 1, sizeof(spaces));
		auto value_end = line.length() - 1;
		while (value_end > value_begin && isspace(line[value_end])) {
			--value_end;
		}

		string name(line, name_begin, name_end - name_begin + 1);
		string value(line, value_begin, value_end - value_begin + 1);

		if (name == "seed") {
			conf.gen.seed = stoul(value);
		} else {
			throw runtime_error("unknown world option: " + name);
		}
	}
	if (in.bad()) {
		throw runtime_error("IO error reading world config");
	}
}

void WorldSave::Write(const World::Config &conf) const {
	if (!make_dirs(root_path)) {
		throw runtime_error("failed to create world save directory");
	}

	ofstream out(conf_path);
	out << "seed = " << conf.gen.seed << endl;
	out.close();

	if (!out) {
		throw runtime_error("failed to write world config");
	}
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
	chunk.InvalidateModel();
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
