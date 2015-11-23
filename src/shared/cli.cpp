#include "CLI.hpp"
#include "CLIContext.hpp"
#include "commands.hpp"

#include "../io/TokenStreamReader.hpp"
#include "../world/Entity.hpp"
#include "../world/Player.hpp"

#include <iostream>
#include <sstream>
#include <glm/gtx/io.hpp>

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

void CLI::Execute(CLIContext &ctx, const string &line) {
	stringstream s(line);
	TokenStreamReader args(s);
	if (!args.HasMore()) {
		// ignore empty command line
		return;
	}
	if (args.Peek().type != Token::IDENTIFIER) {
		ctx.Error("I don't understand");
		return;
	}
	string name;
	args.ReadIdentifier(name);
	auto entry = commands.find(name);
	if (entry == commands.end()) {
		ctx.Error(name + ": command not found");
		return;
	}
	try {
		entry->second->Execute(*this, ctx, args);
	} catch (exception &e) {
		ctx.Error(name + ": " + e.what());
	} catch (...) {
		ctx.Error(name + ": unknown execution error");
	}
}

CLI::Command::~Command() {

}


void TeleportCommand::Execute(CLI &cli, CLIContext &ctx, TokenStreamReader &args) {
	glm::vec3 pos(args.GetFloat(), args.GetFloat(), args.GetFloat());
	EntityState state = ctx.GetPlayer().GetEntity().GetState();
	state.pos = ExactLocation(pos).Sanitize();
	ctx.GetPlayer().GetEntity().SetState(state);

	stringstream msg;
	msg << ctx.GetPlayer().Name() << " teleported to " << pos;
	ctx.Broadcast(msg.str());
}

}
