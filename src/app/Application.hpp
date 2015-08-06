#ifndef BLANK_APP_APPLICATION_HPP_
#define BLANK_APP_APPLICATION_HPP_

#include <SDL.h>
#include <stack>


namespace blank {

class Environment;
class State;
class Window;

class Application {

public:
	explicit Application(Environment &);
	~Application();

	Application(const Application &) = delete;
	Application &operator =(const Application &) = delete;

	/// run until user quits
	void Run();
	/// evaluate a single frame of dt milliseconds
	void Loop(int dt);

	/// run for n frames
	void RunN(size_t n);
	/// run for t milliseconds
	void RunT(size_t t);
	/// run for n frames, assuming t milliseconds for each
	void RunS(size_t n, size_t t);

	/// process all events in SDL's queue
	void HandleEvents();
	void Handle(const SDL_Event &);
	void Handle(const SDL_WindowEvent &);
	/// integrate to the next step with dt milliseconds passed
	void Update(int dt);
	/// push the current state to display
	void Render();

	void PushState(State *);
	State *PopState();
	State *SwitchState(State *);
	State &GetState();
	bool HasState() const noexcept;

private:
	Environment &env;
	std::stack<State *> states;

};

}

#endif
