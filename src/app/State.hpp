#ifndef BLANK_APP_STATE_HPP_
#define BLANK_APP_STATE_HPP_

#include <SDL.h>


namespace blank {

class HeadlessApplication;
class Viewport;

struct State {

	friend class Application;
	friend class HeadlessApplication;

	virtual void Handle(const SDL_Event &) = 0;

	virtual void Update(int dt) = 0;

	virtual void Render(Viewport &) = 0;


private:
	int ref_count = 0;

	virtual void OnEnter() { }
	virtual void OnResume() { }
	virtual void OnPause() { }
	virtual void OnExit() { }

	virtual void OnFocus() { }
	virtual void OnBlur() { }
	virtual void OnResize(Viewport &) { }

};

};

#endif
