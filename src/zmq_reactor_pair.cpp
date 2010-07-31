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

#include "zmq.h"
#include "zmq_reactor_pair.h"
#include "zmq_reactor_uuid.h"

#include <string>
using namespace std;

// todo move to zmq_reactor_pair
// zmq_reactor_policy

// creates unique inproc ZMQ_PAIR, sets socket of reactor, and returns "output" side of pair
void* zmq_reactor_pair(void* context, zmq_reactor_t* reactor)
{
	// create inproc name
	string name = "inproc://uuid_";
	name.append(zmq_reactor_uuid());
	
	// creating socket
	void* bindsock = zmq_socket(context, ZMQ_REQ);
	void* connsock = zmq_socket(context, ZMQ_PAIR);
	
	// errors?
	if (! bindsock || ! connsock) {
		bindsock && zmq_close(bindsock);
		connsock && zmq_close(connsock);
		return 0;
	}
	// bind/connect will only fail if uuid is somehow incompatible with inproc: namespace
	int rc = zmq_bind(bindsock, name.c_str());
	assert(rc == 0);
	
	rc = zmq_connect(connsock, name.c_str());
	assert(rc == 0);
	
	// connect to reactor
	zmq_reactor_connect(reactor, bindsock);
	
	// return the other end (the connect socket)
	return connsock;
}
