#ifndef BLANK_SHARED_COMMANDS_HPP_
#define BLANK_SHARED_COMMANDS_HPP_

#include "CLI.hpp"


namespace blank {

class ImpersonateCommand
: public CLI::Command {

	void Execute(CLI &, CLIContext &, TokenStreamReader &) override;

};

class TeleportCommand
: public CLI::Command {

	void Execute(CLI &, CLIContext &, TokenStreamReader &) override;

};

}

#endif
