#include "Application.hpp"
#include "Assets.hpp"
#include "Environment.hpp"
#include "FrameCounter.hpp"
#include "State.hpp"
#include "StateControl.hpp"

#include "init.hpp"
#include "../audio/Sound.hpp"
#include "../graphics/ArrayTexture.hpp"
#include "../graphics/CubeMap.hpp"
#include "../graphics/Font.hpp"
#include "../graphics/Texture.hpp"
#include "../io/TokenStreamReader.hpp"
#include "../model/bounds.hpp"
#include "../model/Model.hpp"
#include "../model/ModelRegistry.hpp"
#include "../model/Shape.hpp"
#include "../model/ShapeRegistry.hpp"
#include "../shared/ResourceIndex.hpp"
#include "../world/BlockType.hpp"
#include "../world/BlockTypeRegistry.hpp"
#include "../world/Entity.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <SDL_image.h>

using namespace std;


namespace blank {

HeadlessApplication::HeadlessApplication(HeadlessEnvironment &e)
: env(e)
, states() {

}

HeadlessApplication::~HeadlessApplication() {

}


Application::Application(Environment &e)
: HeadlessApplication(e)
, env(e) {

}

Application::~Application() {
	env.audio.StopAll();
}


void HeadlessApplication::RunN(size_t n) {
	Uint32 last = SDL_GetTicks();
	for (size_t i = 0; HasState() && i < n; ++i) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;
		Loop(delta);
		last = now;
	}
}

void HeadlessApplication::RunT(size_t t) {
	Uint32 last = SDL_GetTicks();
	Uint32 finish = last + t;
	while (HasState() && last < finish) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;
		Loop(delta);
		last = now;
	}
}

void HeadlessApplication::RunS(size_t n, size_t t) {
	for (size_t i = 0; HasState() && i < n; ++i) {
		Loop(t);
		cout << '.';
		if (i % 32 == 31) {
			cout << setfill(' ') << setw(5) << right << (i + 1) << endl;
		} else {
			cout << flush;
		}
	}
}


void HeadlessApplication::Run() {
	Uint32 last = SDL_GetTicks();
	while (HasState()) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;
		Loop(delta);
		last = now;
	}
}

void HeadlessApplication::Loop(int dt) {
	env.counter.EnterFrame();
	HandleEvents();
	if (!HasState()) return;
	Update(dt);
	CommitStates();
	if (!HasState()) return;
	env.counter.ExitFrame();
}

void Application::Loop(int dt) {
	env.counter.EnterFrame();
	HandleEvents();
	if (!HasState()) return;
	Update(dt);
	CommitStates();
	if (!HasState()) return;
	Render();
	env.counter.ExitFrame();
}


void HeadlessApplication::HandleEvents() {
	env.counter.EnterHandle();
	SDL_Event event;
	while (HasState() && SDL_PollEvent(&event)) {
		Handle(event);
		CommitStates();
	}
	env.counter.ExitHandle();
}

void HeadlessApplication::Handle(const SDL_Event &event) {
	GetState().Handle(event);
}


void Application::HandleEvents() {
	env.counter.EnterHandle();
	SDL_Event event;
	while (HasState() && SDL_PollEvent(&event)) {
		Handle(event);
		CommitStates();
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
			GetState().OnFocus();
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			GetState().OnBlur();
			break;
		case SDL_WINDOWEVENT_RESIZED:
			env.viewport.Resize(event.data1, event.data2);
			GetState().OnResize(env.viewport);
			break;
		default:
			break;
	}
}

void HeadlessApplication::Update(int dt) {
	env.counter.EnterUpdate();
	if (HasState()) {
		GetState().Update(dt);
	}
	env.counter.ExitUpdate();
}

void Application::Update(int dt) {
	env.counter.EnterUpdate();
	env.audio.Update(dt);
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


void HeadlessApplication::PushState(State *s) {
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

State *HeadlessApplication::PopState() {
	State *s = states.top();
	states.pop();
	s->OnPause();
	s->OnExit();
	if (!states.empty()) {
		states.top()->OnResume();
	}
	return s;
}

State *HeadlessApplication::SwitchState(State *s_new) {
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

State &HeadlessApplication::GetState() {
	return *states.top();
}

void HeadlessApplication::CommitStates() {
	env.state.Commit(*this);
}

bool HeadlessApplication::HasState() const noexcept {
	return !states.empty();
}


void StateControl::Commit(HeadlessApplication &app) {
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
			case POP_AFTER:
				while (app.HasState() && &app.GetState() != m.state) {
					app.PopState();
				}
				break;
			case POP_UNTIL:
				while (app.HasState()) {
					if (app.PopState() == m.state) {
						break;
					}
				}
		}
	}
}


AssetLoader::AssetLoader(const string &base)
: fonts(base + "fonts/")
, sounds(base + "sounds/")
, textures(base + "textures/")
, data(base + "data/") {

}

Assets::Assets(const AssetLoader &loader)
: large_ui_font(loader.LoadFont("DejaVuSans", 24))
, small_ui_font(loader.LoadFont("DejaVuSans", 16)) {

}

void AssetLoader::LoadBlockTypes(
	const string &set_name,
	BlockTypeRegistry &reg,
	ResourceIndex &snd_index,
	ResourceIndex &tex_index,
	const ShapeRegistry &shapes
) const {
	string full = data + set_name + ".types";
	ifstream file(full);
	if (!file) {
		throw runtime_error("failed to open block type file " + full);
	}
	TokenStreamReader in(file);
	string proto;
	while (in.HasMore()) {
		BlockType type;
		in.ReadIdentifier(type.name);
		in.Skip(Token::EQUALS);
		if (in.Peek().type == Token::IDENTIFIER) {
			// prototype
			in.ReadIdentifier(proto);
			type.Copy(reg.Get(proto));
		}
		type.Read(in, snd_index, tex_index, shapes);
		in.Skip(Token::SEMICOLON);
		reg.Add(move(type));
	}
}

CubeMap AssetLoader::LoadCubeMap(const string &name) const {
	string full = textures + name;
	string right = full + "-right.png";
	string left = full + "-left.png";
	string top = full + "-top.png";
	string bottom = full + "-bottom.png";
	string back = full + "-back.png";
	string front = full + "-front.png";

	CubeMap cm;
	cm.Bind();
	SDL_Surface *srf;

	if (!(srf = IMG_Load(right.c_str()))) throw SDLError("IMG_Load");
	try {
		cm.Data(CubeMap::RIGHT, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);

	if (!(srf = IMG_Load(left.c_str()))) throw SDLError("IMG_Load");
	try {
		cm.Data(CubeMap::LEFT, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);

	if (!(srf = IMG_Load(top.c_str()))) throw SDLError("IMG_Load");
	try {
		cm.Data(CubeMap::TOP, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);

	if (!(srf = IMG_Load(bottom.c_str()))) throw SDLError("IMG_Load");
	try {
		cm.Data(CubeMap::BOTTOM, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);

	if (!(srf = IMG_Load(back.c_str()))) throw SDLError("IMG_Load");
	try {
		cm.Data(CubeMap::BACK, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);

	if (!(srf = IMG_Load(front.c_str()))) throw SDLError("IMG_Load");
	try {
		cm.Data(CubeMap::FRONT, *srf);
	} catch (...) {
		SDL_FreeSurface(srf);
		throw;
	}
	SDL_FreeSurface(srf);

	cm.FilterNearest();
	cm.WrapEdge();

	return cm;
}

Font AssetLoader::LoadFont(const string &name, int size) const {
	string full = fonts + name + ".ttf";
	return Font(full.c_str(), size);
}

void AssetLoader::LoadModels(
	const string &set_name,
	ModelRegistry &models,
	ResourceIndex &tex_index,
	const ShapeRegistry &shapes
) const {
	string full = data + set_name + ".models";
	ifstream file(full);
	if (!file) {
		throw runtime_error("failed to open model file " + full);
	}
	TokenStreamReader in(file);
	string model_name;
	string prop_name;
	while (in.HasMore()) {
		in.ReadIdentifier(model_name);
		in.Skip(Token::EQUALS);
		in.Skip(Token::ANGLE_BRACKET_OPEN);
		Model &model = models.Add(model_name);
		while (in.HasMore() && in.Peek().type != Token::ANGLE_BRACKET_CLOSE) {
			in.ReadIdentifier(prop_name);
			in.Skip(Token::EQUALS);
			if (prop_name == "root") {
				model.RootPart().Read(in, tex_index, shapes);
			} else if (prop_name == "body") {
				model.SetBody(in.GetULong());
			} else if (prop_name == "eyes") {
				model.SetEyes(in.GetULong());
			} else {
				while (in.HasMore() && in.Peek().type != Token::SEMICOLON) {
					in.Next();
				}
			}
			in.Skip(Token::SEMICOLON);
		}
		model.Enumerate();
		in.Skip(Token::ANGLE_BRACKET_CLOSE);
		in.Skip(Token::SEMICOLON);
	}
}

void AssetLoader::LoadShapes(const string &set_name, ShapeRegistry &shapes) const {
	string full = data + set_name + ".shapes";
	ifstream file(full);
	if (!file) {
		throw runtime_error("failed to open shape file " + full);
	}
	TokenStreamReader in(file);
	string shape_name;
	while (in.HasMore()) {
		in.ReadIdentifier(shape_name);
		in.Skip(Token::EQUALS);
		Shape &shape = shapes.Add(shape_name);
		shape.Read(in);
		in.Skip(Token::SEMICOLON);
	}
}

Sound AssetLoader::LoadSound(const string &name) const {
	string full = sounds + name + ".wav";
	return Sound(full.c_str());
}

Texture AssetLoader::LoadTexture(const string &name) const {
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

void AssetLoader::LoadTexture(const string &name, ArrayTexture &tex, int layer) const {
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

void AssetLoader::LoadTextures(const ResourceIndex &index, ArrayTexture &tex) const {
	// TODO: where the hell should that size come from?
	tex.Reserve(16, 16, index.Size(), Format());
	for (const auto &entry : index.Entries()) {
		LoadTexture(entry.first, tex, entry.second);
	}
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

	//Print(cout);

	sum = Frame<int>();
	max = Frame<int>();
}

void FrameCounter::Print(ostream &out) const {
	out << fixed << right << setprecision(2) << setfill(' ')
		<< "PEAK handle: " << setw(2) << peak.handle
		<< ".00ms, update: " << setw(2) << peak.update
		<< ".00ms, render: " << setw(2) << peak.render
		<< ".00ms, running: " << setw(2) << peak.running
		<< ".00ms, waiting: " << setw(2) << peak.waiting
		<< ".00ms, total: " << setw(2) << peak.total
		<< ".00ms" << endl
		<< " AVG handle: " << setw(5) << avg.handle
		<< "ms, update: " << setw(5) << avg.update
		<< "ms, render: " << setw(5) << avg.render
		<< "ms, running: " << setw(5) << avg.running
		<< "ms, waiting: " << setw(5) << avg.waiting
		<< "ms, total: " << setw(5) << avg.total
		<< "ms" << endl;
}

}
