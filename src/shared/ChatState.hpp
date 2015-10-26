#ifndef BLANK_SHARED_CHATSTATE_HPP_
#define BLANK_SHARED_CHATSTATE_HPP_

#include "../app/State.hpp"

#include "../ui/TextInput.hpp"

#include <string>


namespace blank {

class Environment;

class ChatState
: public State {

public:
	struct Responder {
		virtual void OnLineSubmit(const std::string &) = 0;
	};

public:
	ChatState(Environment &env, State &parent, Responder &);

	void Preset(const std::string &);
	void Clear();

	void OnResume() override;
	void OnPause() override;

	void OnResize(Viewport &) override;

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

	void Quit();

private:
	Environment &env;
	State &parent;
	Responder &responder;

	std::string preset;
	TextInput input;

};

}

#endif
