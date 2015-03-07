#ifndef BLANK_HUD_H_
#define BLANK_HUD_H_

#include "model.hpp"
#include "shader.hpp"
#include "world.hpp"

#include <glm/glm.hpp>


namespace blank {

class HUD {

public:
	HUD();

	HUD(const HUD &) = delete;
	HUD &operator =(const HUD &) = delete;

	void Viewport(float width, float height);
	void Viewport(float x, float y, float width, float height);

	void Display(const BlockType &);

	void Render(DirectionalLighting &);

private:
	Model block;
	glm::mat4 block_transform;
	bool block_visible;

	OutlineModel crosshair;
	glm::mat4 crosshair_transform;

	float near, far;
	glm::mat4 projection;
	glm::mat4 view;

};

}

#endif
