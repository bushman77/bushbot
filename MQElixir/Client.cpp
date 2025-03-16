#include "Client.h"
#include <mq/Plugin.h>

Client::Client()
{
	WriteChatf("Client object created!");
}

Client::~Client()
{
	WriteChatf("Client object destroyed!");
}

void Client::PrintHelloWorld()
{
	WriteChatf("Hello, World! From Client class!");
}
