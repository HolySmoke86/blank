#ifndef BLANK_STANDALONE_DIRECTCLIFEEDBACK_HPP_
#define BLANK_STANDALONE_DIRECTCLIFEEDBACK_HPP_

#include "../shared/CLIContext.hpp"


namespace blank {

class HUD;

namespace standalone {

class DirectCLIFeedback
: public CLIContext {

public:
	DirectCLIFeedback(Player &, HUD &);

	void Error(const std::string &) override;
	void Message(const std::string &) override;
	void Broadcast(const std::string &) override;

private:
	HUD &hud;

};

}
}

#endif
