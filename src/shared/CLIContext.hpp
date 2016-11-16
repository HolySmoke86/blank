#ifndef BLANK_SHARED_CLICONTEXT_HPP_
#define BLANK_SHARED_CLICONTEXT_HPP_

#include <string>


namespace blank {

class Player;
class Entity;

class CLIContext {

public:
	/// Create context with optional player and entity
	/// entity defaults to the player's if given
	/// Associated player or entity can be changed during
	/// the context's lifetime and will assume the original
	/// values when reset.
	explicit CLIContext(Player *p = nullptr, Entity *e = nullptr);

	/// check if this context associates a player
	bool HasPlayer() { return effective_player; }
	/// get the player responsible for all this
	/// only valid if HasPlayer() returns true
	Player &GetPlayer() { return *effective_player; }
	/// change the effective player of this context
	/// note that this will *not* change the effective entity
	void SetPlayer(Player &p) { effective_player = &p; }

	/// check if this context associates an entity
	bool HasEntity() { return effective_entity; }
	/// get the entity on which operations should be performed
	/// only valid if HasPlayer() returns true
	Entity &GetEntity() { return *effective_entity; }
	/// change the effective player of this context
	void SetEntity(Entity &e) { effective_entity = &e; }

	/// reset effective player and entity to their original values
	void Reset() {
		effective_player = original_player;
		effective_player = original_player;
	}

	/// an error has happened and the player should be notified
	virtual void Error(const std::string &) = 0;

	/// return to sender
	/// use this for output concerning the originator of a command
	virtual void Message(const std::string &) = 0;

	/// send a status message to all players
	/// use this to announce stuff which may be interesting to anyone
	virtual void Broadcast(const std::string &) = 0;

private:
	Player *original_player;
	Player *effective_player;
	Entity *original_entity;
	Entity *effective_entity;

};

}

#endif
