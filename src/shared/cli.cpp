#include "CLI.hpp"
#include "CLIContext.hpp"
#include "commands.hpp"

#include "../io/TokenStreamReader.hpp"
#include "../world/Entity.hpp"
#include "../world/Player.hpp"
#include "../world/World.hpp"

#include <iostream>
#include <sstream>
#include <glm/gtx/io.hpp>

using namespace std;


namespace blank {

CLI::CLI(World &world)
: world(world)
, commands() {
	AddCommand("as", new ImpersonateCommand);
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


CLIContext::CLIContext(Player *p, Entity *e)
: original_player(p)
, effective_player(p)
, original_entity(e)
, effective_entity(e) {
	if (!e && p) {
		original_entity = effective_entity = &p->GetEntity();
	}
}

std::string CLIContext::Name() const {
	if (HasPlayer()) return GetPlayer().Name();
	if (HasEntity()) return GetEntity().Name();
	return "anonymous";
}


void ImpersonateCommand::Execute(CLI &cli, CLIContext &ctx, TokenStreamReader &args) {
	if (!args.HasMore()) {
		// no argument => reset
		ctx.Reset();
		ctx.Broadcast(ctx.Name() + " returned to their own self");
		return;
	}
	// TODO: broadcast who (real player name) impersonates who
	string old_name = ctx.Name();
	string name(args.GetString());

	Player *p = cli.GetWorld().FindPlayer(name);
	if (p) {
		ctx.SetPlayer(*p);
		ctx.SetEntity(p->GetEntity());
		ctx.Broadcast(old_name + " now impersonating " + p->Name());
		return;
	}

	// not a player, try an entity
	Entity *e = cli.GetWorld().FindEntity(name);
	if (e) {
		ctx.SetEntity(*e);
		ctx.Broadcast(old_name + " now impersonating " + e->Name());
		return;
	}

	ctx.Error("no player or entity with name " + name);
}


void TeleportCommand::Execute(CLI &, CLIContext &ctx, TokenStreamReader &args) {
	if (!ctx.HasEntity()) {
		ctx.Error("teleport needs entity to operate on");
		return;
	}

	glm::vec3 pos(args.GetFloat(), args.GetFloat(), args.GetFloat());
	EntityState state = ctx.GetEntity().GetState();
	state.pos = ExactLocation(pos).Sanitize();
	ctx.GetEntity().SetState(state);

	stringstream msg;
	msg << ctx.GetEntity().Name() << " teleported to " << pos;
	ctx.Broadcast(msg.str());
}

}
