#include "hud.hpp"

#include "init.hpp"
#include "shape.hpp"

#include <glm/gtc/matrix_transform.hpp>


namespace blank {

HUD::HUD()
: block()
, block_transform(1.0f)
, block_visible(false)
, crosshair()
, crosshair_transform(1.0f)
, near(100.0f)
, far(-100.0f)
, projection(glm::ortho(0.0f, 1.0f, 1.0f, 0.0f, near, far))
, view(glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0))) {
	block_transform = glm::translate(block_transform, glm::vec3(35.0f, 80.0f, 0.0f));
	block_transform = glm::scale(block_transform, glm::vec3(50.0f));
	block_transform = glm::rotate(block_transform, 3.5f, glm::vec3(1.0f, 0.0f, 0.0f));
	block_transform = glm::rotate(block_transform, 0.85f, glm::vec3(0.0f, 1.0f, 0.0f));

	crosshair.vertices = std::vector<glm::vec3>({
		{ -10.0f,   0.0f, 0.0f }, { 10.0f,  0.0f, 0.0f },
		{   0.0f, -10.0f, 0.0f }, {  0.0f, 10.0f, 0.0f },
	});
	crosshair.colors.resize(4, { 10.0f, 10.0f, 10.0f });
	crosshair.Invalidate();
}


void HUD::Viewport(float width, float height) {
	Viewport(0, 0, width, height);
}

void HUD::Viewport(float x, float y, float width, float height) {
	projection = glm::ortho(x, width, height, y, near, far);
	crosshair_transform = glm::translate(glm::mat4(1.0f), glm::vec3(width * 0.5f, height * 0.5f, 0.0f));
}


void HUD::Display(const BlockType &type) {
	block.Clear();
	type.FillModel({ 0.0f, 0.0f, 0.0f }, block);
	block_visible = type.visible;
}


void HUD::Render(DirectionalLighting &program) {
	if (block_visible) {
		program.SetLightDirection({ 1.0f, 3.0f, 5.0f });
		GLContext::ClearDepthBuffer();
		program.SetMVP(block_transform, view, projection);
		block.Draw();
		program.SetM(crosshair_transform);
		crosshair.Draw();
	}
}

}
