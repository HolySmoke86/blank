#ifndef BLANK_UI_HUD_H_
#define BLANK_UI_HUD_H_

#include "../graphics/Texture.hpp"
#include "../model/Model.hpp"
#include "../model/OutlineModel.hpp"
#include "../model/SpriteModel.hpp"

#include <glm/glm.hpp>


namespace blank {

class BlendedSprite;
class Block;
class BlockTypeRegistry;
class DirectionalLighting;
class Font;

class HUD {

public:
	HUD(const BlockTypeRegistry &, const Font &);

	HUD(const HUD &) = delete;
	HUD &operator =(const HUD &) = delete;

	void Viewport(float width, float height) noexcept;
	void Viewport(float x, float y, float width, float height) noexcept;

	void Display(const Block &);

	void Render(DirectionalLighting &, BlendedSprite &) noexcept;

private:
	const BlockTypeRegistry &types;
	const Font &font;

	Model block;
	Model::Buffer block_buf;
	glm::mat4 block_transform;

	Texture block_label;
	SpriteModel label_sprite;
	glm::mat4 label_transform;
	SDL_Color label_color;

	bool block_visible;

	OutlineModel crosshair;
	glm::mat4 crosshair_transform;

	float near, far;
	glm::mat4 projection;
	glm::mat4 view;

};

}

#endif
