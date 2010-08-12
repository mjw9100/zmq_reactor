
#include "server.hpp"

#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <zmq.hpp>

#include <iostream>
#include <sstream>
using namespace std;

void *server_thread(void *arg)
{
    zmq::context_t *pctx = reinterpret_cast<zmq::context_t*>(arg);
	assert(pctx != NULL);
	
    //  Worker thread is set up as a requester so the first packet can be initiated by the server
    zmq::socket_t s(*pctx, ZMQ_REQ);
	
    //  Connect to the dispatcher (queue) running in the main thread.
    s.connect("inproc://servers");

	// say hello
	zmq::message_t hello;
	s.send(hello);
	
    for (;;) {
		zmq::message_t request;
        //  Get a request from the dispatcher.
        s.recv(&request);
		
		// make sure there's no more
		int64_t more = 0;
		size_t more_size = sizeof(more);
		s.getsockopt(ZMQ_RCVMORE, &more, &more_size);
		assert(more == 0);
		
		// simulate service delay
		sleep(1);
		
		ostringstream o;
		o.write(reinterpret_cast<const char*>(request.data()), request.size());
		o << " SERVER " << pthread_self();
		zmq::message_t reply(o.str().size());
		memcpy(reinterpret_cast<char*>(reply.data()), o.str().c_str(), reply.size());
		s.send(reply);
//		cout << "SERVER SENT REPLY" << endl;
    }
}

void start_server(zmq::context_t* pctx, unsigned n)
{
	assert(pctx != NULL);
	
    //  Launch 10 worker threads.
    for (unsigned i = 0; i < n; ++i) {
        pthread_t worker;
        int rc = pthread_create(&worker, NULL, server_thread, pctx);
        assert (rc == 0);
    }
}
