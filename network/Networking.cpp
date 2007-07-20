#include "Networking.h"


namespace Networking {
    const std::string DISCOVERY_QUESTION = "Yo, can I play Free-O here, dog?";
    const std::string DISCOVERY_ANSWER = "Word!";
    const int DISCOVERY_PORT = 12345;
    const int MESSAGE_PORT = 12346;
    const int SOCKET_LINGER_TIME = 1 << (sizeof(unsigned short) * 8 - 1);
    const int HOST_PLAYER_ID = 0;
    const int INVALID_PLAYER_ID = -1;
}
