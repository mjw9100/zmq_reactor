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

#include "server.hpp"

#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <zmq.hpp>

#include <iostream>
using namespace std;

void *server_thread(void *arg)
{
    zmq::context_t *pctx = static_cast<zmq::context_t*>(arg);
	assert(pctx != NULL);
	
    //  Worker thread is a 'replier', i.e. it receives requests and returns replies.
    zmq::socket_t s(*pctx, ZMQ_REP);
	
    //  Connect to the dispatcher (queue) running in the main thread.
    s.connect("inproc://workers");
	
    for (;;) {
        //  Get a request from the dispatcher.
        zmq::message_t request;
        s.recv (&request);
		
		// make sure there's no more
		int64_t more = 0;
		size_t more_size = sizeof(more);
		s.getsockopt(ZMQ_RCVMORE, &more, &more_size);
		assert(more == 0);
		
		// fake work
		//sleep(1);
		
		// modify reply
		zmq::message_t reply(request.size() + 1);
		char* cp = static_cast<char*>(reply.data());
		*cp++ = 'X';
		memcpy(cp, request.data(), request.size());
		s.send(reply);
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
