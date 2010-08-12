
#include "client.hpp"
#include "server.hpp"

#include <iostream>
#include <stdexcept>
using namespace std;

#include "req_server.hpp"

int main(int argc, char** argv)
{
	zmq::context_t ctx(1);
	
	zmq::socket_t servers(ctx, ZMQ_XREP);
	servers.bind("inproc://servers");
		
	zmq::socket_t clients(ctx, ZMQ_XREP);
	clients.bind("inproc://clients");
		
	start_server(&ctx, 3);
	
	start_client(&ctx, 5);
	
	req_server(clients, servers);
	
	return 0;
}