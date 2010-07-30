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
