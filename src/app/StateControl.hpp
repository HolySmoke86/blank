#ifndef BLANK_APP_STATECONTROL_HPP_
#define BLANK_APP_STATECONTROL_HPP_

#include <queue>


namespace blank {

class HeadlessApplication;
class State;

class StateControl {

public:
	// add state to the front
	void Push(State *s) {
		cue.emplace(PUSH, s);
	}

	// swap state at the front
	void Switch(State *s) {
		cue.emplace(SWITCH, s);
	}

	// remove state at the front
	void Pop() {
		cue.emplace(POP);
	}

	// remove all states
	// application will exit if nothing is pushed after this
	void PopAll() {
		cue.emplace(POP_ALL);
	}

	// pop states until this one is on top
	void PopAfter(State *s) {
		cue.emplace(POP_AFTER, s);
	}

	// pop states until this one is removed
	void PopUntil(State *s) {
		cue.emplace(POP_UNTIL, s);
	}


	void Commit(HeadlessApplication &);

private:
	enum Command {
		PUSH,
		SWITCH,
		POP,
		POP_ALL,
		POP_AFTER,
		POP_UNTIL,
	};
	struct Memo {
		State *state;
		Command cmd;
		explicit Memo(Command c, State *s = nullptr): state(s), cmd(c) { }
	};
	std::queue<Memo> cue;

};

}

#endif
