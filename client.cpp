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
