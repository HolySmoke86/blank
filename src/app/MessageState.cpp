#include "MessageState.hpp"

#include "Environment.hpp"


namespace blank {

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
	if (e.type == SDL_KEYDOWN) {
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

}
