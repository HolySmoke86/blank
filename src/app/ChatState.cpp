#include "ChatState.hpp"

#include "Environment.hpp"
#include "../io/event.hpp"

#include <iostream>


namespace blank {

ChatState::ChatState(Environment &env, State &parent, Responder &responder)
: env(env)
, parent(parent)
, responder(responder)
, input(env.assets.small_ui_font) {
	input.Position(glm::vec3(25.0f, -25.0f, -1.0f), Gravity::SOUTH_WEST, Gravity::SOUTH_WEST);
	input.Width(env.viewport.Width() - 50.0f);
	input.Foreground(glm::vec4(1.0f));
	input.Background(glm::vec4(0.5f));
}

void ChatState::OnResume() {
	OnResize(env.viewport);
	input.Clear();
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

}
