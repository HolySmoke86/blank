#include "ChatState.hpp"
#include "MessageState.hpp"
#include "ProgressState.hpp"

#include "../app/Environment.hpp"
#include "../io/event.hpp"

#include <iostream>


namespace blank {

ChatState::ChatState(Environment &env, State &parent, Responder &responder)
: env(env)
, parent(parent)
, responder(responder)
, preset()
, input(env.assets.small_ui_font) {
	input.Position(glm::vec3(25.0f, -25.0f, -1.0f), Gravity::SOUTH_WEST, Gravity::SOUTH_WEST);
	input.Width(env.viewport.Width() - 50.0f);
	input.Foreground(PrimitiveMesh::Color(255));
	input.Background(PrimitiveMesh::Color(127));
}

void ChatState::Preset(const std::string &text) {
	preset = text;
}

void ChatState::Clear() {
	preset.clear();
}

void ChatState::OnResume() {
	OnResize(env.viewport);
	input.Clear();
	if (!preset.empty()) {
		input.Insert(preset.c_str());
	}
	input.Focus(env.viewport);
}

void ChatState::OnPause() {
	input.Blur();
}

void ChatState::OnResize(Viewport &viewport) {
	input.Width(viewport.Width() - 50.0f);
}


void ChatState::Handle(const SDL_Event &e) {
	switch (e.type) {
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {
				case SDLK_ESCAPE:
					Quit();
					break;
				case SDLK_KP_ENTER:
				case SDLK_RETURN:
					responder.OnLineSubmit(input.GetInput());
					Quit();
					break;

				case SDLK_BACKSPACE:
					input.Backspace();
					break;
				case SDLK_DELETE:
					input.Delete();
					break;

				case SDLK_LEFT:
					input.MoveBackward();
					break;
				case SDLK_RIGHT:
					input.MoveForward();
					break;

				case SDLK_HOME:
					input.MoveBegin();
					break;
				case SDLK_END:
					input.MoveEnd();
					break;

				default:
					break;
			}
			break;

		case SDL_QUIT:
			env.state.PopAll();
			break;

		case SDL_TEXTINPUT:
			input.Handle(e.text);
			break;

		case SDL_TEXTEDITING:
			std::cout << e << std::endl;
			input.Handle(e.edit);
			break;

		default:
			break;
	}
}

void ChatState::Quit() {
	env.state.PopUntil(this);
}

void ChatState::Update(int dt) {
	parent.Update(dt);
}

void ChatState::Render(Viewport &viewport) {
	parent.Render(viewport);
	input.Render(viewport);
}


MessageState::MessageState(Environment &env)
: env(env) {
	message.Position(glm::vec3(0.0f), Gravity::CENTER);
	message.Hide();
	press_key.Position(glm::vec3(0.0f, env.assets.large_ui_font.LineSkip(), 0.0f), Gravity::CENTER);
	press_key.Set(env.assets.small_ui_font, "press any key to continue");
	press_key.Show();
}

void MessageState::SetMessage(const char *msg) {
	message.Set(env.assets.large_ui_font, msg);
	message.Show();
}

void MessageState::ClearMessage() {
	message.Hide();
}

void MessageState::Handle(const SDL_Event &e) {
	if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN) {
		env.state.Pop();
	}
}

void MessageState::Update(int dt) {

}

void MessageState::Render(Viewport &viewport) {
	if (message.Visible()) {
		message.Render(viewport);
	}
	if (press_key.Visible()) {
		press_key.Render(viewport);
	}
}


ProgressState::ProgressState(Environment &env, const char *tpl)
: env(env)
, progress(env.assets.large_ui_font) {
	progress.Position(glm::vec3(0.0f), Gravity::CENTER);
	progress.Template(tpl);
}

void ProgressState::SetProgress(int value, int total) {
	progress.Update(value, total);
}

void ProgressState::Handle(const SDL_Event &e) {
	if (e.type == SDL_QUIT) {
		env.state.PopAll();
	}
}

void ProgressState::Render(Viewport &viewport) {
	progress.Render(viewport);
}

}
