#ifndef BLANK_APP_APPLICATION_HPP_
#define BLANK_APP_APPLICATION_HPP_

#include <SDL.h>
#include <stack>


namespace blank {

class Environment;
class HeadlessEnvironment;
class State;
class Window;

class HeadlessApplication {

public:
	explicit HeadlessApplication(HeadlessEnvironment &);
	~HeadlessApplication();

	void PushState(State *);
	State *PopState();
	State *SwitchState(State *);
	State &GetState();
	void CommitStates();
	bool HasState() const noexcept;

	/// run until out of states
	void Run();
	/// evaluate a single frame of dt milliseconds
	virtual void Loop(int dt);

	/// run for n frames
	void RunN(size_t n);
	/// run for t milliseconds
	void RunT(size_t t);
	/// run for n frames, assuming t milliseconds for each
	void RunS(size_t n, size_t t);

	/// process all events in SDL's queue
	void HandleEvents();
	void Handle(const SDL_Event &);
	/// integrate to the next step with dt milliseconds passed
	void Update(int dt);

private:
	HeadlessEnvironment &env;
	std::stack<State *> states;

};


class Application
: public HeadlessApplication {

public:
	explicit Application(Environment &);
	~Application();

	Application(const Application &) = delete;
	Application &operator =(const Application &) = delete;

	void Loop(int dt) override;

	/// process all events in SDL's queue
	void HandleEvents();
	void Handle(const SDL_Event &);
	void Handle(const SDL_WindowEvent &);
	/// integrate to the next step with dt milliseconds passed
	void Update(int dt);
	/// push the current state to display
	void Render();

private:
	Environment &env;

};

}

#endif
