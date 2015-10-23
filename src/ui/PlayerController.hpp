#ifndef BLANK_UI_PLAYERCONTROLLER_HPP_
#define BLANK_UI_PLAYERCONTROLLER_HPP_

#include <glm/glm.hpp>

#include "../world/EntityCollision.hpp"
#include "../world/EntityController.hpp"
#include "../world/WorldCollision.hpp"


namespace blank {

class Player;
class World;

class PlayerController
: public EntityController {

public:
	PlayerController(World &, Player &);

	World &GetWorld() noexcept { return world; }
	const World &GetWorld() const noexcept { return world; }
	Player &GetPlayer() noexcept { return player; }
	const Player &GetPlayer() const noexcept { return player; }

	WorldCollision &BlockFocus() noexcept { return aim_world; }
	const WorldCollision &BlockFocus() const noexcept { return aim_world; }
	EntityCollision &EntityFocus() noexcept { return aim_entity; }
	const EntityCollision &EntityFocus() const noexcept { return aim_entity; }

	/// set desired direction of movement
	/// the magnitude (clamped to [0..1]) can be used to attenuate target velocity
	void SetMovement(const glm::vec3 &) noexcept;
	const glm::vec3 &GetMovement() const noexcept { return move_dir; }

	glm::vec3 ControlForce(const Entity &, const EntityState &) const override;

	/// turn the controlled entity's head by given pitch and yaw deltas
	void TurnHead(float pitch, float yaw) noexcept;

	/// get player pitch in radians, normalized to [-PI/2,PI/2]
	float GetPitch() const noexcept;
	/// get player yaw in radians, normalized to [-PI,PI]
	float GetYaw() const noexcept;

	/// start doing primary action
	/// what exactly this means depends on the active item
	virtual void StartPrimaryAction() = 0;
	/// stop doing primary action
	virtual void StopPrimaryAction() = 0;
	// etc
	virtual void StartSecondaryAction() = 0;
	virtual void StopSecondaryAction() = 0;
	virtual void StartTertiaryAction() = 0;
	virtual void StopTertiaryAction() = 0;

	/// set the item at given inventory slot as active
	void SelectInventory(int) noexcept;
	int InventorySlot() const noexcept;

protected:
	void Invalidate() noexcept;
	void UpdatePlayer() noexcept;

private:
	World &world;
	Player &player;
	glm::vec3 move_dir;
	bool dirty;

	WorldCollision aim_world;
	EntityCollision aim_entity;

};

}

#endif
