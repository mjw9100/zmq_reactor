/*
 * ----------------------------------------------------------
 *
 * Copyright 2010 [x+1] Solutions, Inc.
 *
 * 470 Park Avenue South, Suite 7N
 * New York, NY  10016
 *
 * ----------------------------------------------------------
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */

#include <zmq.hpp>
#include <string.h>
#include <iostream>
#include <sstream>
using namespace std;

void *client_thread(void *arg)
{
    zmq::context_t *pctx = static_cast<zmq::context_t*>(arg);
	assert(pctx != NULL);

	// create socket
    zmq::socket_t* psocket(new zmq::socket_t(*pctx, ZMQ_REQ));
	assert(psocket != NULL);
	
    //  Connect to the dispatcher (queue) running in the main thread.
    psocket->connect("inproc://server");
	
	unsigned request_id = 0;
	
    for (;;) {
		// format request
		ostringstream o;
		o << "thread# " << pthread_self() << " request# " << ++request_id;
 		const string request = o.str();
		
		// send request
		zmq::message_t msg(request.size());
		memcpy(msg.data(), request.c_str(), request.size());
		assert(psocket->send(msg) == true);
		
        //  Get the reply. 
        zmq::message_t reply;
        assert(psocket->recv(&reply) == true);

		cout.write(static_cast<const char*>(reply.data()), reply.size()) << endl;

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
