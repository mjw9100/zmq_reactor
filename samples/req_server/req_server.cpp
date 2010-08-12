
#include "req_server.hpp"

#include <iostream>
#include <list>
#include <vector>
#include <string>
using namespace std;

#include "boost/unordered_map.hpp"

#include "zmq.hpp"

#include "zmq_reactor.h"

// for now
#define	zmq_assert		assert
#define	errno_assert	assert

class req_server_impl {
	// free_list_t - list of inactive servers
	// active_map_t	- map from active server to client
	typedef std::vector<zmq_reactor_t> reactors_t;
	typedef std::list<string> servers_t;
	typedef boost::unordered_map<string,string> active_t;

	// common msg
	zmq_msg_t	msg_;
	
	// client / server sockets
	void* client_;
	void* server_;

	reactors_t	reactors_;
	servers_t	available_;
	active_t	active_;

	// proxies
	static int server_proxy(void* socket, short revents, struct zmq_reactor_t*, void* hint)
	{
		return ((req_server_impl*)hint)->server_handler(socket);
	}
	
	static int client_proxy(void* socket, short revents, struct zmq_reactor_t*, void* hint)
	{
		return ((req_server_impl*)hint)->client_handler(socket);
	}
	

public:
	
	req_server_impl(void* client, void* server) : client_(client), server_(server)
	{
		int rc = zmq_msg_init(&msg_);
		assert(rc == 0);
	}
	
	int server_handler(void* socket)
	{
		// this is where we get called by zmq_reactor
		enum { S_IDENT, S_DELIM, S_FIRSTMSG, S_OTHER };
		int state = S_IDENT;
		
		// pipe names
		string server_pipe;
		string client_pipe;
		
		for (;;) {
			// receive message
			int rc = zmq_recv(socket, &msg_, 0);
			errno_assert (rc == 0);
			
			// get ZMQ_RCVMORE
			int64_t more;
			size_t moresz = sizeof(more);
			rc = zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &moresz);
			errno_assert(rc == 0);
			
			// handle identity packet
			switch (state) {
				case S_IDENT:
				{
					// get message size
					size_t size = zmq_msg_size(&msg_);
					assert(size != 0);
					
					// get server pipe name
					const char* cp = reinterpret_cast<const char*>(zmq_msg_data(&msg_));
					copy(cp, cp + size, back_inserter<string>(server_pipe));
					
					// lookup up corresponding client
					active_t::iterator pclient = active_.find(server_pipe);
					
					// whether or not this is a match, the server is available
					available_.push_back(server_pipe);
					
					// enable client
					zmq_reactor_events(&reactors_[0], ZMQ_POLLIN);
					
					// rewrite address to client_pipe
					if (pclient != active_.end()) {
						// get client pipe
						client_pipe = pclient->second;
						//cout << "FOUND CLIENT PIPE " << client_pipe << endl;
						// remove server from active list
						active_.erase(pclient);
						// rebuild message
						assert(zmq_msg_close(&msg_) == 0);
						assert(zmq_msg_init_size(&msg_, client_pipe.size()) == 0);
						memcpy(zmq_msg_data(&msg_), client_pipe.c_str(), zmq_msg_size(&msg_));
						//cout << "server: " << server_pipe << " client: " << client_pipe << endl;
					}
					++state;
					break;
				}
				case S_FIRSTMSG:
					if (zmq_msg_size(&msg_) != 0 && client_pipe.empty())
						cerr << "WARNING: no client pipe for server: " << server_pipe << endl;
					++state;
					break;
				case S_DELIM:
					++state;
				// no default
			}
			
			// send on client pipe
			if (! client_pipe.empty()) {
				//					cout << "SENDING ON CLIENT PIPE" << endl;
				rc = zmq_send(client_, &msg_, more ? ZMQ_SNDMORE : 0);
				errno_assert(rc == 0);
			}
			
			if (!more)
				break;
		}
		return 0;
	}
	
	int client_handler(void* socket)
	{
		int64_t more = 0;
		
		// NB only recv if there are available servers
		if (available_.empty()) {
			// disable client
			zmq_reactor_events(&reactors_[0], 0);
		}
		else {
			enum { S_IDENT, S_OTHER };
			int state = S_IDENT;
			
			// server available - pop server pipe
			string server_pipe = available_.front();
			available_.pop_front();
			
			for (;;) {
				// receive message
				int rc = zmq_recv(socket, &msg_, 0);
				errno_assert(rc == 0);
				
				// get more flag
				size_t moresz = sizeof(more);
				rc = zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &moresz);
				errno_assert(rc == 0);
				
				size_t size = zmq_msg_size(&msg_);
				
				if (state == S_IDENT) {
					// identity packet size != 0
					assert(size != 0);
					
					// get client pipe name
					string client_pipe;
					const char* begin = reinterpret_cast<const char*>(zmq_msg_data(&msg_));
					copy(begin, begin + size, back_inserter<string>(client_pipe));
					
					// insert mapping from server to client
					assert(active_.insert(make_pair(server_pipe, client_pipe)).second);
					
					// rewrite packet
					assert(zmq_msg_close(&msg_) == 0);
					assert(zmq_msg_init_size(&msg_, server_pipe.size()) == 0);
					memcpy(zmq_msg_data(&msg_), server_pipe.c_str(), zmq_msg_size(&msg_));
					
					// bump state
					++state;
				}
				
				// send message
				rc = zmq_send(server_, &msg_, more ? ZMQ_SNDMORE : 0);
				errno_assert(rc == 0);
				
				if (!more)
					break;
			}
		}
		return 0;
	}
	
	int device()
	{
		reactors_.resize(2);
		zmq_reactor_init_socket(&reactors_[0], client_, ZMQ_POLLIN, client_proxy, this);
		zmq_reactor_init_socket(&reactors_[1], server_, ZMQ_POLLIN, server_proxy, this);
		
		// disable client initially
		zmq_reactor_events(&reactors_[0], 0);
		
		int rc = zmq_reactor_poll(&reactors_[0], reactors_.size(), -1);
		assert(rc == 0);
		return rc;
	}
};

int req_server(void* client, void* server)
{
	int rc = req_server_impl(client, server).device();
	
	return rc;
}


