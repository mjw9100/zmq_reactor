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

#include "queue_device.hpp"

#include "zmq_reactor.h"
#include "zmq.h"

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
	
	// poll
	int rc = zmq_reactor_poll(&reactor1, -1);
	
	// done
	return rc;
}

