#include "WorldSave.hpp"

#include "../app/io.hpp"

#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace std;


namespace blank {

WorldSave::WorldSave(const string &path)
: root_path(path)
, conf_path(path + "world.conf") {

}


bool WorldSave::Exists() const noexcept {
	return is_dir(root_path) && is_file(conf_path);
}


void WorldSave::Create(const World::Config &conf) const {
	cout << "creating world save" << endl;

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

void WorldSave::Read(World::Config &conf) const {
	cout << "reading world save" << endl;

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

}
