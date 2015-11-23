#ifndef BLANK_SERVER_NETWORKCLIFEEDBACK_HPP_
#define BLANK_SERVER_NETWORKCLIFEEDBACK_HPP_

#include "../shared/CLIContext.hpp"


namespace blank {

namespace server {

class ClientConnection;

class NetworkCLIFeedback
: public CLIContext {

public:
	NetworkCLIFeedback(Player &, ClientConnection &);

	void Error(const std::string &) override;
	void Message(const std::string &) override;
	void Broadcast(const std::string &) override;

private:
	ClientConnection &conn;

};

}
}

#endif
