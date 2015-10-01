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

	void Update(int dt);

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

	IntervalTimer place_timer;
	IntervalTimer remove_timer;

};

}

#endif
