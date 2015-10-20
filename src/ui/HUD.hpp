#ifndef BLANK_UI_HUD_H_
#define BLANK_UI_HUD_H_

#include "FixedText.hpp"
#include "MessageBox.hpp"
#include "../graphics/EntityMesh.hpp"
#include "../graphics/PrimitiveMesh.hpp"

#include <glm/glm.hpp>


namespace blank {

class Block;
class BlockTypeRegistry;
class Config;
class Environment;
class Font;
class Player;
class Viewport;

class HUD {

public:
	explicit HUD(Environment &, Config &, const Player &);

	HUD(const HUD &) = delete;
	HUD &operator =(const HUD &) = delete;

	// focus
	void FocusBlock(const Chunk &, int);
	void FocusEntity(const Entity &);
	void FocusNone();

	// "inventory"
	void DisplayNone();
	void Display(const BlockType &);

	// debug overlay
	void UpdateDebug();
	void UpdateCounter();
	void UpdatePosition();
	void UpdateOrientation();

	// message box
	void PostMessage(const char *);
	void PostMessage(const std::string &msg) {
		PostMessage(msg.c_str());
	}

	void Update(int dt);
	void Render(Viewport &) noexcept;

private:
	Environment &env;
	Config &config;
	const Player &player;

	// block focus
	PrimitiveMesh outline;
	glm::mat4 outline_transform;
	bool outline_visible;

	// "inventory"
	EntityMesh block;
	EntityMesh::Buffer block_buf;
	glm::mat4 block_transform;
	FixedText block_label;
	bool block_visible;

	// debug overlay
	FixedText counter_text;
	FixedText position_text;
	FixedText orientation_text;
	FixedText block_text;
	FixedText entity_text;
	bool show_block;
	bool show_entity;

	// message box
	MessageBox messages;
	IntervalTimer msg_timer;

	// crosshair
	PrimitiveMesh crosshair;

};

}

#endif
