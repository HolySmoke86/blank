#ifndef BLANK_UI_HUD_H_
#define BLANK_UI_HUD_H_

#include "../graphics/Texture.hpp"
#include "../model/Model.hpp"
#include "../model/OutlineModel.hpp"
#include "../model/SpriteModel.hpp"

#include <glm/glm.hpp>


namespace blank {

class Block;
class BlockTypeRegistry;
class Font;
class Viewport;

class HUD {

public:
	HUD(const BlockTypeRegistry &, const Font &);

	HUD(const HUD &) = delete;
	HUD &operator =(const HUD &) = delete;

	void Display(const Block &);

	void Render(Viewport &) noexcept;

private:
	const BlockTypeRegistry &types;
	const Font &font;

	Model block;
	Model::Buffer block_buf;
	glm::mat4 block_transform;

	Texture block_label;
	SpriteModel label_sprite;
	glm::mat4 label_transform;

	bool block_visible;

	OutlineModel crosshair;

};

}

#endif
