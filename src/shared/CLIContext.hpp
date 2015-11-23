#ifndef BLANK_SHARED_CLICONTEXT_HPP_
#define BLANK_SHARED_CLICONTEXT_HPP_

#include <string>


namespace blank {

class Player;

class CLIContext {

public:
	explicit CLIContext(Player &p)
	: player(p) { }

	/// get the player responsible for all this
	Player &GetPlayer() { return player; }

	/// an error has happened and the player should be notified
	virtual void Error(const std::string &) = 0;

	/// return to sender
	/// use this for output concerning the originator of a command
	virtual void Message(const std::string &) = 0;

	/// send a status message to all players
	/// use this to announce stuff which may be interesting to anyone
	virtual void Broadcast(const std::string &) = 0;

private:
	Player &player;

};

}

#endif
