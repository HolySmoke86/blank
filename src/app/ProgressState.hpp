#ifndef BLANK_APP_PROGRESSSTATE_HPP_
#define BLANK_APP_PROGRESSSTATE_HPP_

#include "State.hpp"

#include "../ui/Progress.hpp"


namespace blank {

class Environment;

class ProgressState
: public State {

public:
	ProgressState(Environment &env, const char *tpl);

	void SetProgress(int value, int total);

	void Handle(const SDL_Event &) override;
	void Render(Viewport &) override;

private:
	Environment &env;
	Progress progress;

};

}

#endif
