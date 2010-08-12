
#include <zmq.hpp>
#include <string.h>
#include <iostream>
#include <sstream>
using namespace std;


void *client_thread(void *arg)
{
    zmq::context_t *pctx = reinterpret_cast<zmq::context_t*>(arg);
	assert(pctx != NULL);
	
	// create socket
    zmq::socket_t* psocket(new zmq::socket_t(*pctx, ZMQ_REQ));
	assert(psocket != NULL);
	
    //  Connect to the dispatcher (queue) running in the main thread.
    psocket->connect("inproc://clients");
	
	unsigned request_id = 0;
	
    for (;;) {
		// format request
		ostringstream o;
		o << "CLIENT " << pthread_self() << " REQUEST# " << ++request_id;
 		const string request = o.str();
		
		//		cout << "client send: " << request << endl;
		// send request
		zmq::message_t msg(request.size());
		memcpy(msg.data(), request.c_str(), request.size());
		assert(psocket->send(msg) == true);
		
        //  Get the reply. 
        zmq::message_t reply;
        assert(psocket->recv(&reply) == true);
		
		if (reply.size() == 0) 
			cout << "CLIENT RECEIVED EMPTY PACKET" << endl;
		else
			cout.write(reinterpret_cast<const char*>(reply.data()), reply.size()) << endl;
	}
}


void start_client(zmq::context_t* pctx, unsigned n)
{
	assert(pctx != NULL);
	
	for (unsigned i = 0; i < n; ++i) {
		pthread_t worker;
		int rc = pthread_create(&worker, NULL, client_thread, pctx);
		assert (rc == 0);
	}
}
