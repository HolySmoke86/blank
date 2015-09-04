#ifndef BLANK_CLIENT_INITIALSTATE_HPP_
#define BLANK_CLIENT_INITIALSTATE_HPP_

#include "../app/State.hpp"
#include "../ui/FixedText.hpp"


namespace blank {
namespace client {

class MasterState;

class InitialState
: public State {

public:
	explicit InitialState(MasterState &);

	void OnEnter() override;

	void Handle(const SDL_Event &) override;
	void Update(int dt) override;
	void Render(Viewport &) override;

private:
	MasterState &master;
	FixedText message;

};

}
}

#endif
