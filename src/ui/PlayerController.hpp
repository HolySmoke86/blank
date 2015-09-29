#ifndef BLANK_UI_PLAYERCONTROLLER_HPP_
#define BLANK_UI_PLAYERCONTROLLER_HPP_

#include <glm/glm.hpp>


namespace blank {

struct PlayerController {

	/// set desired direction of movement
	/// the magnitude (clamped to [0..1]) can be used to attenuate target velocity
	virtual void SetMovement(const glm::vec3 &) = 0;
	/// turn the controlled entity's head by given pitch and yaw deltas
	virtual void TurnHead(float pitch, float yaw) = 0;

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
	virtual void SelectInventory(int) = 0;

};

}

#endif
