#ifndef BLANK_UI_DIRECTINPUT_HPP_
#define BLANK_UI_DIRECTINPUT_HPP_

#include "PlayerController.hpp"

#include "../app/IntervalTimer.hpp"


namespace blank {

class Player;
class World;
struct WorldManipulator;

class DirectInput
: public PlayerController {

public:
	DirectInput(World &, Player &, WorldManipulator &);

	void Update(Entity &, float dt) override;

	void StartPrimaryAction() override;
	void StopPrimaryAction() override;
	void StartSecondaryAction() override;
	void StopSecondaryAction() override;
	void StartTertiaryAction() override;
	void StopTertiaryAction() override;

private:
	void PickBlock();
	void PlaceBlock();
	void RemoveBlock();

private:
	WorldManipulator &manip;

	FineTimer place_timer;
	FineTimer remove_timer;

};

}

#endif
