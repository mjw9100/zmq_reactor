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

#include "queue_device.hpp"

#include "zmq.h"
#include "zmq_reactor.h"
#include "zmq_reactor_ops.h"

#include <cassert>

static int queue_handler(void* socket, short events, zmq_reactor_t* reactor, void* hint)
{
	zmq_msg_t	msg_;
	int rc = zmq_msg_init(&msg_);
	assert(rc == 0);
	
	for (;;) {
		// receive message
		int rc = zmq_recv(socket, &msg_, 0);
		assert(rc == 0);
		
		int64_t more;
		size_t moresz = sizeof(more);
		
		rc = zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &moresz);
		assert(rc == 0);
		
		rc = zmq_send(hint, &msg_, more ? ZMQ_SNDMORE : 0);
		assert(rc == 0);
		
		if (!more)
			break;
	}
	
	return 0;
}

int queue_device(void* socket1, void* socket2)
{
	zmq_reactor_t reactor1, reactor2;
	
	assert(zmq_reactor_init_socket(&reactor1, socket1, ZMQ_POLLIN, queue_handler, socket2) == 0);
	assert(zmq_reactor_init_socket(&reactor2, socket2, ZMQ_POLLIN, queue_handler, socket1) == 0);
	
	// assemble queue
	zmq_reactor_insert(&reactor1, &reactor2);

	// unnecessary, but demonstrative
	zmq_reactor_ops_normal(&reactor1);
	
	// poll
	int rc = zmq_reactor_poll(&reactor1, -1);
	return rc;
}

