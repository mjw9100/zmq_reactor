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
	pr->next	= pr;
	pr->prev	= pr;
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
// zmq_reactor_insert - insert a new reactor
//
void zmq_reactor_insert(zmq_reactor_t* where, zmq_reactor_t* pr)
{
	assert(where != NULL && pr != NULL);
	assert(pr->next == pr && pr->prev == pr);
	std::swap(pr->next, where->prev->next);
	std::swap(pr->prev, where->prev);
}


//
// zmq_reactor_remove - unlinks reactor
//
void zmq_reactor_remove(zmq_reactor_t* pr)
{
	assert(pr != NULL);
	std::swap(pr->next, pr->prev->next);
	std::swap(pr->prev, pr->prev->next->prev);
	assert(pr->next == pr && pr->prev == pr);
}


//
// zmq_reactor_close - unlinks reactor and closes socket
//
int zmq_reactor_close(zmq_reactor_t* pr)
{
	assert(pr != NULL);
	zmq_reactor_remove(pr);
	int rc = zmq_close(pr->socket);
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

short zmq_reactor_ops(zmq_reactor_t* pr, short ops)
{
	assert(pr != NULL);
	std::swap(ops, pr->ops);
	return ops;
}

//
// build_pollitems (static)
//
static zmq_pollitem_t* build_pollitems(zmq_reactor_t* preactor, zmq_pollitem_t* pitems, zmq_reactor_t** preactors)
{
	for (zmq_reactor_t* curp = preactor;;) {
		// save reactor address for this index
		*preactors		= curp;
		// setup poll items
		pitems->socket	= curp->socket;
		pitems->fd		= 0;
		pitems->events	= curp->events;
		pitems->revents	= 0;
		// bump list
		++preactors;
		++pitems;
		
		// go to next item
		curp = curp->next;
		if (curp == preactor)
			break;
	}
	
	return pitems;
}


//
// refresh_pollitems (static)
//
static void refresh_pollitems(zmq_pollitem_t* begin, zmq_pollitem_t* end, zmq_reactor_t** preactor)
{
	for (; begin != end; ++begin, ++preactor)
		begin->events = (*preactor)->events;
}

//
// zmq_reactor_repoll_policy
//
int zmq_reactor_poll(zmq_reactor_t* begin, const long utimeout)
{
	// TODO: is std::vector efficient here?
	zmq_reactor_t* reactors[MAX_POLL];
	zmq_pollitem_t items[MAX_POLL];
	
	assert(begin != NULL);
	
	// initialize pollitems
	zmq_pollitem_t* end = build_pollitems(begin, items, reactors);
	
	// starting point
	zmq_pollitem_t* start = items;
	long timeout = utimeout; 

	for (;;) {
		// update polling vectors
		refresh_pollitems(start, end, reactors);

		IF_DEBUG(cout << "zmq_poll offset=" << (start - items) << " timeout=" << timeout << endl);
		// poll
		int rc = 0;
		rc = zmq_poll(start, end - start, timeout);

		IF_DEBUG(cout << "zmq_poll rc=" << rc << endl);

		// reset start
		zmq_pollitem_t* pitem = items;
		swap(start, pitem);
		timeout = utimeout;
		
		// timeout was set to 0 by immediate poll
		if (rc == 0 && utimeout)
			continue;
		
		// rc == 0 if timeout reached, or if no items were enabled for polling
		if (rc == -1 || rc == 0)
			return rc;
		
		assert(rc > 0);
		
		
		for (; pitem != end; ++pitem) {
			zmq_reactor_t* pr = reactors[pitem - items];
			if (pitem->revents || pr->ops & ZMQ_REACTOR_OPS_ALWAYS) {
				IF_DEBUG(cout << "calling handler " << (pitem - items) << endl);
				rc = (pr->handler)(pitem->socket, pitem->revents, pr, pr->hint);
				// force return if non-zero
				if (rc)
					return rc;
				
				// get cmd
				if (pr->ops & ZMQ_REACTOR_OPS_INSTR) {
					short instr = pr->ops & ZMQ_REACTOR_OPS_INSTR;
					short offset = pr->ops & ZMQ_REACTOR_OPS_OFFSET;
				
					if (instr == ZMQ_REACTOR_OPS_EXIT) {
						return offset;
					}
					else if (instr == ZMQ_REACTOR_OPS_POLA) {
						if (offset >= end - items)
							return -1;
						start = items + offset;
						break;
					}
					else if (instr == ZMQ_REACTOR_OPS_POLFI) {
						if (offset >= end - pitem)
							return -1;
						start = pitem + offset;
						timeout = 0;
						break;
					}
					else if (instr == ZMQ_REACTOR_OPS_BRF) {
						if (offset >= end - pitem)
							return -1;
						// move pitem forward
						pitem += offset;
					}
				}
			}
		}
	}
}


//
