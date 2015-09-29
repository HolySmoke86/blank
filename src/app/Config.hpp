#ifndef BLANK_APP_CONFIG_HPP_
#define BLANK_APP_CONFIG_HPP_

#include <cstdint>
#include <iosfwd>
#include <string>


namespace blank {

struct Config {

	struct Audio {

		bool enabled = true;

	} audio;

	struct Input {

		bool keyboard = true;
		bool mouse = true;

		float pitch_sensitivity = -0.0025f;
		float yaw_sensitivity = -0.001f;

	} input;

	struct Network {

		std::string host = "localhost";
		std::uint16_t port = 12354;

	} net;

	struct Player {

		std::string name = "default";

	} player;

	struct Video {

		bool dblbuf = true;
		bool vsync = true;
		int msaa = 1;

		bool hud = true;
		bool world = true;
		bool debug = false;

	} video;

	void Load(std::istream &);
	void Save(std::ostream &);

};

}

#endif
