#include "Application.hpp"
#include "Assets.hpp"
#include "Environment.hpp"
#include "FrameCounter.hpp"
#include "State.hpp"
#include "StateControl.hpp"

#include "init.hpp"
#include "../audio/Sound.hpp"
#include "../graphics/ArrayTexture.hpp"
#include "../graphics/Font.hpp"
#include "../graphics/Texture.hpp"
#include "../world/BlockType.hpp"
#include "../world/Entity.hpp"

#include <iostream>
#include <stdexcept>
#include <SDL_image.h>

using std::string;


namespace blank {

Application::Application(Environment &e)
: env(e)
, states() {

}

Application::~Application() {
	env.audio.StopAll();
}


void Application::RunN(size_t n) {
	Uint32 last = SDL_GetTicks();
	for (size_t i = 0; HasState() && i < n; ++i) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;
		Loop(delta);
		last = now;
	}
}

void Application::RunT(size_t t) {
	Uint32 last = SDL_GetTicks();
	Uint32 finish = last + t;
	while (HasState() && last < finish) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;
		Loop(delta);
		last = now;
	}
}

void Application::RunS(size_t n, size_t t) {
	for (size_t i = 0; HasState() && i < n; ++i) {
		Loop(t);
	}
}


void Application::Run() {
	Uint32 last = SDL_GetTicks();
	env.window.GrabMouse();
	while (HasState()) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;
		Loop(delta);
		last = now;
	}
}

void Application::Loop(int dt) {
	env.counter.EnterFrame();
	HandleEvents();
	if (!HasState()) return;
	Update(dt);
	env.state.Commit(*this);
	if (!HasState()) return;
	Render();
	env.counter.ExitFrame();
}


void Application::HandleEvents() {
	env.counter.EnterHandle();
	SDL_Event event;
	while (HasState() && SDL_PollEvent(&event)) {
		Handle(event);
		env.state.Commit(*this);
	}
	env.counter.ExitHandle();
}

void Application::Handle(const SDL_Event &event) {
	switch (event.type) {
		case SDL_WINDOWEVENT:
			Handle(event.window);
			break;
		default:
			GetState().Handle(event);
			break;
	}
}

void Application::Handle(const SDL_WindowEvent &event) {
	switch (event.event) {
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			env.window.GrabMouse();
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			env.window.ReleaseMouse();
			break;
		case SDL_WINDOWEVENT_RESIZED:
			env.viewport.Resize(event.data1, event.data2);
			break;
		default:
			break;
	}
}

void Application::Update(int dt) {
	env.counter.EnterUpdate();
	if (HasState()) {
		GetState().Update(dt);
	}
	env.counter.ExitUpdate();
}

void Application::Render() {
	// gl implementation may (and will probably) delay vsync blocking until
	// the first write after flipping, which is this clear call
	env.viewport.Clear();
	env.counter.EnterRender();

	if (HasState()) {
		GetState().Render(env.viewport);
	}

	env.counter.ExitRender();
	env.window.Flip();
}


void Application::PushState(State *s) {
	if (!states.empty()) {
		states.top()->OnPause();
	}
	states.emplace(s);
	++s->ref_count;
	if (s->ref_count == 1) {
		s->OnEnter();
	}
	s->OnResume();
}

State *Application::PopState() {
	State *s = states.top();
	states.pop();
	s->OnPause();
	s->OnExit();
	if (!states.empty()) {
		states.top()->OnResume();
	}
	return s;
}

State *Application::SwitchState(State *s_new) {
	State *s_old = states.top();
	states.top() = s_new;
	--s_old->ref_count;
	++s_new->ref_count;
	s_old->OnPause();
	if (s_old->ref_count == 0) {
		s_old->OnExit();
	}
	if (s_new->ref_count == 1) {
		s_new->OnEnter();
	}
	s_new->OnResume();
	return s_old;
}

State &Application::GetState() {
	return *states.top();
}

bool Application::HasState() const noexcept {
	return !states.empty();
}


void StateControl::Commit(Application &app) {
	while (!cue.empty()) {
		Memo m(cue.front());
		cue.pop();
		switch (m.cmd) {
			case PUSH:
				app.PushState(m.state);
				break;
			case SWITCH:
				app.SwitchState(m.state);
				break;
			case POP:
				app.PopState();
				break;
			case POP_ALL:
				while (app.HasState()) {
					app.PopState();
				}
				break;
		}
	}
}


Assets::Assets(const string &base)
: fonts(base + "fonts/")
, sounds(base + "sounds/")
, textures(base + "textures/") {

}

Font Assets::LoadFont(const string &name, int size) const {
	string full = fonts + name + ".ttf";
	return Font(full.c_str(), size);
}

Sound Assets::LoadSound(const string &name) const {
	string full = sounds + name + ".wav";
	return Sound(full.c_str());
}

Texture Assets::LoadTexture(const string &name) const {
	string full = textures + name + ".png";
	Texture tex;
	SDL_Surface *srf = IMG_Load(full.c_str());
	if (!srf) {
		throw SDLError("IMG_Load");
	}
	tex.Bind();
	tex.Data(*srf);
	SDL_FreeSurface(srf);
	return tex;
}

void Assets::LoadTexture(const string &name, ArrayTexture &tex, int layer) const {
	string full = textures + name + ".png";
	SDL_Surface *srf = IMG_Load(full.c_str());
	if (!srf) {
		throw SDLError("IMG_Load");
	}
	tex.Bind();
	try {
		tex.Data(layer, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);
}


void FrameCounter::EnterFrame() noexcept {
	last_enter = SDL_GetTicks();
	last_tick = last_enter;
}

void FrameCounter::EnterHandle() noexcept {
	Tick();
}

void FrameCounter::ExitHandle() noexcept {
	current.handle = Tick();
}

void FrameCounter::EnterUpdate() noexcept {
	Tick();
}

void FrameCounter::ExitUpdate() noexcept {
	current.update = Tick();
}

void FrameCounter::EnterRender() noexcept {
	Tick();
}

void FrameCounter::ExitRender() noexcept {
	current.render = Tick();
}

void FrameCounter::ExitFrame() noexcept {
	Uint32 now = SDL_GetTicks();
	current.total = now - last_enter;
	current.running = current.handle + current.update + current.render;
	current.waiting = current.total - current.running;
	Accumulate();

	++cur_frame;
	if (cur_frame >= NUM_FRAMES) {
		Push();
		cur_frame = 0;
		changed = true;
	} else {
		changed = false;
	}
}

int FrameCounter::Tick() noexcept {
	Uint32 now = SDL_GetTicks();
	int delta = now - last_tick;
	last_tick = now;
	return delta;
}

void FrameCounter::Accumulate() noexcept {
	sum.handle += current.handle;
	sum.update += current.update;
	sum.render += current.render;
	sum.running += current.running;
	sum.waiting += current.waiting;
	sum.total += current.total;

	max.handle = std::max(current.handle, max.handle);
	max.update = std::max(current.update, max.update);
	max.render = std::max(current.render, max.render);
	max.running = std::max(current.running, max.running);
	max.waiting = std::max(current.waiting, max.waiting);
	max.total = std::max(current.total, max.total);

	current = Frame<int>();
}

void FrameCounter::Push() noexcept {
	peak = max;
	avg.handle = sum.handle * factor;
	avg.update = sum.update * factor;
	avg.render = sum.render * factor;
	avg.running = sum.running * factor;
	avg.waiting = sum.waiting * factor;
	avg.total = sum.total * factor;

	sum = Frame<int>();
	max = Frame<int>();
}

}
