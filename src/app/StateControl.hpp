#ifndef BLANK_APP_STATECONTROL_HPP_
#define BLANK_APP_STATECONTROL_HPP_

#include <queue>


namespace blank {

class Application;
class State;

class StateControl {

public:
	void Push(State *s) {
		cue.emplace(PUSH, s);
	}

	void Switch(State *s) {
		cue.emplace(SWITCH, s);
	}

	void Pop() {
		cue.emplace(POP);
	}

	void PopAll() {
		cue.emplace(POP_ALL);
	}


	void Commit(Application &);

private:
	enum Command {
		PUSH,
		SWITCH,
		POP,
		POP_ALL,
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
