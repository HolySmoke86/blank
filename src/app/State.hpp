#ifndef BLANK_APP_STATE_HPP_
#define BLANK_APP_STATE_HPP_

#include <SDL.h>


namespace blank {

class Viewport;

struct State {

	virtual void Handle(const SDL_Event &) = 0;

	virtual void Update(int dt) = 0;

	virtual void Render(Viewport &) = 0;

};

};

#endif
