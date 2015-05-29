#ifndef BLANK_HUD_H_
#define BLANK_HUD_H_

#include "model.hpp"
#include "world.hpp"

#include <glm/glm.hpp>


namespace blank {

class BlockTypeRegistry;
class DirectionalLighting;

class HUD {

public:
	explicit HUD(const BlockTypeRegistry &);

	HUD(const HUD &) = delete;
	HUD &operator =(const HUD &) = delete;

	void Viewport(float width, float height) noexcept;
	void Viewport(float x, float y, float width, float height) noexcept;

	void Display(const Block &);

	void Render(DirectionalLighting &) noexcept;

private:
	const BlockTypeRegistry &types;

	Model block;
	Model::Buffer block_buf;
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
