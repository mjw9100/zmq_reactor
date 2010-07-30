/*
 * ----------------------------------------------------------
 *
 * Copyright 2010 [x+1] Solutions, Inc. All Rights Reserved.
 *
 * 470 Park Avenue South, Suite 7N
 * New York, NY  10016
 *
 * ----------------------------------------------------------
 * 
 * Permission is granted to redistribute this module and/or modify it under
 * the terms of the Lesser GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 * 
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
