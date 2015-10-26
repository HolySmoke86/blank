#ifndef BLANK_SHARED_CLI_HPP_
#define BLANK_SHARED_CLI_HPP_

#include <map>
#include <string>


namespace blank {

class Player;
class TokenStreamReader;
class World;

class CLI {

public:
	struct Command {
		virtual ~Command();
		virtual void Execute(CLI &, Player &, TokenStreamReader &) = 0;
	};

public:
	explicit CLI(World &);
	~CLI();

	void AddCommand(const std::string &name, Command *);

	void Execute(Player &, const std::string &);

	void Message(const std::string &msg);
	void Error(const std::string &msg);

private:
	World &world;
	std::map<std::string, Command *> commands;

};

}

#endif
