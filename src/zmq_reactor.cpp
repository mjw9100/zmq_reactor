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

#include "zmq_reactor.h"

#include "zmq.h"

#include <cassert>
#include <iostream>
using namespace std;

//#define	ZMQ_REACTOR_DEBUG

#ifdef	ZMQ_REACTOR_DEBUG
#define	IF_DEBUG(N)	N
#else
#define	IF_DEBUG(N)
#endif//ZMQ_REACTOR_DEBUG

// Arbitrary but reasonable?
enum { MAX_POLL = 50 };

//
// zmq_reactor_init - initialize reactor
//
int zmq_reactor_init(zmq_reactor_t* pr, short events, zmq_reactor_handler_t* handler, void* hint)
{
	assert(pr != NULL);
	pr->ops		= 0;
	pr->events	= events;
	pr->socket	= 0;
	pr->handler	= handler;
	pr->hint	= hint;
	return 0;
}


//
// zmq_reactor_init_socket - initialize reactor with specific zmq socket
//
int zmq_reactor_init_socket(zmq_reactor_t* pr, void* socket, short events, zmq_reactor_handler_t* handler, void* hint)
{
	zmq_reactor_init(pr, events, handler, hint);
	pr->socket = socket;
	return 0;
}


//
// zmq_reactor_close - unlinks reactor and closes socket
//
int zmq_reactor_close(zmq_reactor_t* pr)
{
	assert(pr != NULL);
	int rc = 0;
	if (pr->socket)
		rc = zmq_close(pr->socket);
	return rc;
}


//
// zmq_reactor_connect - sets new socket, returns old socket
//
void* zmq_reactor_connect(zmq_reactor_t* pr, void* socket)
{
	assert(pr != NULL);
	std::swap(socket, pr->socket);
	return socket;
}


//
// zmq_reactor_socket - returns current socket
//
void* zmq_reactor_socket(zmq_reactor_t* pr)
{
	assert(pr != NULL);
	return pr->socket;
}


//
// zmq_reactor_events
//
short zmq_reactor_events(zmq_reactor_t* pr, short events)
{
	assert(pr != NULL);
	std::swap(events, pr->events);
	return events;
}


//
// zmq_reactor_ops
//
short zmq_reactor_ops(zmq_reactor_t* pr, short ops)
{
	assert(pr != NULL);
	std::swap(ops, pr->ops);
	return ops;
}


//
// STATIC build_pollitems
//
static void build_pollitems(int nitems, zmq_pollitem_t* items, zmq_reactor_t* reactors)
{
	for (zmq_pollitem_t* end = items + nitems; items != end; ++items, ++reactors) {
		// setup poll items
		items->socket	= reactors->socket;
		items->fd		= 0;
		items->events	= reactors->events;
		items->revents	= 0;
	}
}


//
// STATIC refresh_pollitems
//
static void refresh_pollitems(int nitems, zmq_pollitem_t* items, zmq_reactor_t* reactors)
{
	for (zmq_pollitem_t* end = items + nitems; items != end; ++items, ++reactors)
		items->events = reactors->events;
}

//
// zmq_reactor_repoll_policy
//
int zmq_reactor_poll(zmq_reactor_t* reactors, int nitems, const long utimeout)
{
	// TODO: is std::vector efficient here?
	zmq_pollitem_t items[MAX_POLL];

	if (nitems < 1 || nitems > MAX_POLL) {
		errno = ERANGE;
		return -1;
	}
	
	assert(reactors != NULL);
	
	// initialize pollitems
	build_pollitems(nitems, items, reactors);
	
	// starting point
	int offset = 0;
	long timeout = utimeout; 

	for (;;) {
		IF_DEBUG(cout << "zmq_poll offset=" << offset << " timeout=" << timeout << endl);

		// update polling vectors
		refresh_pollitems(nitems - offset, items + offset, reactors + offset);

		// poll
		int rc = 0;
		rc = zmq_poll(items + offset, nitems - offset, timeout);

		IF_DEBUG(cout << "zmq_poll rc=" << rc << endl);

		// reset start
		int curoff = 0;
		swap(offset, curoff);
		timeout = utimeout;
		
		// timeout was set to 0 by immediate poll
		if (rc == 0 && utimeout)
			continue;
		
		// rc == 0 if timeout reached, or if no items were enabled for polling
		if (rc == -1 || rc == 0)
			return rc;
		
		assert(rc > 0);
		
		zmq_pollitem_t* pi = items + curoff;
		zmq_reactor_t* pr = reactors + curoff;
		for (zmq_pollitem_t* end = items + nitems; pi != end; ++pi, ++pr) {
			if (pi->revents || pr->ops & ZMQ_REACTOR_OPS_ALWAYS) {
				IF_DEBUG(cout << "calling handler " << (pi - items) << endl);
				rc = (pr->handler)(pi->socket, pi->revents, pr, pr->hint);
				// force return if non-zero
				if (rc)
					return rc;
				
				// get cmd
				if (pr->ops & ZMQ_REACTOR_OPS_INSTR) {
					short instr = pr->ops & ZMQ_REACTOR_OPS_INSTR;
					short opoff = pr->ops & ZMQ_REACTOR_OPS_OFFSET;
				
					if (instr == ZMQ_REACTOR_OPS_EXIT) {
						return opoff;
					}
					else if (instr == ZMQ_REACTOR_OPS_POLA) {
						if (opoff >= nitems)
							return -1;
						offset = opoff;
						break;
					}
					else if (instr == ZMQ_REACTOR_OPS_POLFI) {
						if (opoff >= end - pi)
							return -1;
						offset = (pi - items) + opoff;
						timeout = 0;
						break;
					}
					else if (instr == ZMQ_REACTOR_OPS_BRF) {
						if (opoff >= end - pi)
							return -1;
						// move pitem forward
						pi += opoff;
						pr += opoff;
					}
				}
			}
		}
	}
}


//
