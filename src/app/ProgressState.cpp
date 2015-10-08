#include "ProgressState.hpp"

#include "../app/Environment.hpp"


namespace blank {

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
