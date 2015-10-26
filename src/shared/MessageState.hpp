#ifndef BLANK_SHARED_MESSAGESTATE_HPP_
#define BLANK_SHARED_MESSAGESTATE_HPP_

#include "../app/State.hpp"

#include "../ui/FixedText.hpp"


namespace blank {

class Environment;

class MessageState
: public State {

public:
	explicit MessageState(Environment &);

	void SetMessage(const char *);
	void ClearMessage();

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

private:
	Environment &env;
	FixedText message;
	FixedText press_key;

};

}

#endif
