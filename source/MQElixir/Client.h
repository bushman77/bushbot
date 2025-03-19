#ifndef CLIENT_H
#define CLIENT_H

#include <string>

class Client {
public:
    Client();
    ~Client();

    void connect(const std::string& serverAddress, int port);
    void join(const std::string& channel);
    void disconnect();
};

#endif // CLIENT_H
