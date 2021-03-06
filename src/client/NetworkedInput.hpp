#ifndef BLANK_CLIENT_NETWORKEDINPUT_HPP_
#define BLANK_CLIENT_NETWORKEDINPUT_HPP_

#include "../ui/PlayerController.hpp"

#include "../world/EntityState.hpp"

#include <cstdint>
#include <list>


namespace blank {
namespace client {

class Client;

class NetworkedInput
: public PlayerController {

public:
	explicit NetworkedInput(World &, Player &, Client &);

	bool UpdateImportant() const noexcept;
	void Update(Entity &, float dt) override;
	void PushPlayerUpdate(int dt);
	void MergePlayerCorrection(std::uint16_t, const EntityState &);

	void StartPrimaryAction() override;
	void StopPrimaryAction() override;
	void StartSecondaryAction() override;
	void StopSecondaryAction() override;
	void StartTertiaryAction() override;
	void StopTertiaryAction() override;

private:
	Client &client;

	struct PlayerHistory {
		EntityState state;
		glm::vec3 movement;
		float delta_t;
		std::uint16_t packet;
		PlayerHistory(EntityState s, const glm::vec3 &mv, float dt, std::uint16_t p)
		: state(s), movement(mv), delta_t(dt), packet(p) { }
	};
	std::list<PlayerHistory> player_hist;

	glm::vec3 old_movement;

	std::uint8_t old_actions;
	std::uint8_t actions;

};

}
}

#endif
