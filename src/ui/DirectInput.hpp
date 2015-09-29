#ifndef BLANK_UI_DIRECTINPUT_HPP_
#define BLANK_UI_DIRECTINPUT_HPP_

#include "PlayerController.hpp"

#include "../app/IntervalTimer.hpp"
#include "../world/EntityCollision.hpp"
#include "../world/WorldCollision.hpp"


namespace blank {

class Player;
class World;
struct WorldManipulator;

class DirectInput
: public PlayerController {

public:
	DirectInput(World &, Player &, WorldManipulator &);

	const WorldCollision &BlockFocus() const noexcept { return aim_world; }
	const EntityCollision &EntityFocus() const noexcept { return aim_entity; }

	void Update(int dt);

	void SetMovement(const glm::vec3 &) override;
	void TurnHead(float pitch, float yaw) override;
	void StartPrimaryAction() override;
	void StopPrimaryAction() override;
	void StartSecondaryAction() override;
	void StopSecondaryAction() override;
	void StartTertiaryAction() override;
	void StopTertiaryAction() override;
	void SelectInventory(int) override;

private:
	void UpdatePlayer();

	void PickBlock();
	void PlaceBlock();
	void RemoveBlock();

private:
	World &world;
	Player &player;
	WorldManipulator &manip;

	WorldCollision aim_world;
	EntityCollision aim_entity;

	glm::vec3 move_dir;
	float pitch;
	float yaw;
	bool dirty;

	IntervalTimer place_timer;
	IntervalTimer remove_timer;

};

}

#endif
