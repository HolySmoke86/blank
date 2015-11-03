#include "CLI.hpp"
#include "commands.hpp"

#include "../io/TokenStreamReader.hpp"
#include "../world/Entity.hpp"
#include "../world/Player.hpp"

#include <iostream>
#include <sstream>

using namespace std;


namespace blank {

CLI::CLI(World &world)
: world(world)
, commands() {
	AddCommand("tp", new TeleportCommand);
}

CLI::~CLI() {
	for (auto &entry : commands) {
		delete entry.second;
	}
}

void CLI::AddCommand(const string &name, Command *cmd) {
	commands[name] = cmd;
}

void CLI::Execute(Player &player, const string &line) {
	stringstream s(line);
	TokenStreamReader args(s);
	if (!args.HasMore()) {
		// ignore empty command line
		return;
	}
	if (args.Peek().type != Token::IDENTIFIER) {
		Error("I don't understand");
		return;
	}
	string name;
	args.ReadIdentifier(name);
	auto entry = commands.find(name);
	if (entry == commands.end()) {
		Error(name + ": command not found");
		return;
	}
	try {
		entry->second->Execute(*this, player, args);
	} catch (exception &e) {
		Error(name + ": " + e.what());
	} catch (...) {
		Error(name + ": unknown execution error");
	}
}

void CLI::Message(const string &msg) {
	// TODO: display message to player
	cout << msg << endl;
}

void CLI::Error(const string &msg) {
	Message("CLI error: " + msg);
}

CLI::Command::~Command() {

}


void TeleportCommand::Execute(CLI &cli, Player &player, TokenStreamReader &args) {
	glm::vec3 pos(args.GetFloat(), args.GetFloat(), args.GetFloat());
	EntityState state = player.GetEntity().GetState();
	state.pos = ExactLocation(pos).Sanitize();
	player.GetEntity().SetState(state);
}

}
