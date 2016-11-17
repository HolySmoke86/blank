#ifndef BLANK_SHARED_CLI_HPP_
#define BLANK_SHARED_CLI_HPP_

#include <map>
#include <string>


namespace blank {

class CLIContext;
class TokenStreamReader;
class World;

class CLI {

public:
	struct Command {
		virtual ~Command();
		virtual void Execute(CLI &, CLIContext &, TokenStreamReader &) = 0;
	};

public:
	explicit CLI(World &);
	~CLI();

	void AddCommand(const std::string &name, Command *);

	void Execute(CLIContext &, const std::string &);

	World &GetWorld() noexcept { return world; }

private:
	World &world;
	std::map<std::string, Command *> commands;

};

}

#endif
