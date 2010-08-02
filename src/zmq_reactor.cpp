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


// Arbitrary but reasonable?
enum { MAX_POLL = 50 };


//
// zmq_reactor_init - initialize reactor
//
int zmq_reactor_init(zmq_reactor_t* preactor, short events, zmq_reactor_handler_t* handler, void* hint)
{
	assert(preactor != NULL);
	preactor->next		= preactor;
	preactor->prev		= preactor;
	preactor->options	= 0;
	preactor->events	= events;
	preactor->socket	= 0;
	preactor->handler	= handler;
	preactor->hint		= hint;
	return 0;
}


//
// zmq_reactor_init_socket - initialize reactor with specific zmq socket
//
int zmq_reactor_init_socket(zmq_reactor_t* preactor, void* socket, short events, zmq_reactor_handler_t* handler, void* hint)
{
	zmq_reactor_init(preactor, events, handler, hint);
	preactor->socket = socket;
	return 0;
}


//
// zmq_reactor_insert - insert a new reactor
//
void zmq_reactor_insert(zmq_reactor_t* where, zmq_reactor_t* preactor)
{
	assert(where != NULL && preactor != NULL);
	assert(preactor->next == preactor && preactor->prev == preactor);
	std::swap(preactor->next, where->prev->next);
	std::swap(preactor->prev, where->prev);
}


//
// zmq_reactor_remove - unlinks reactor
//
void zmq_reactor_remove(zmq_reactor_t* preactor)
{
	assert(preactor != NULL);
	std::swap(preactor->next, preactor->prev->next);
	std::swap(preactor->prev, preactor->prev->next->prev);
	assert(preactor->next == preactor && preactor->prev == preactor);
}


//
// zmq_reactor_close - unlinks reactor and closes socket
//
int zmq_reactor_close(zmq_reactor_t* preactor)
{
	assert(preactor != NULL);
	zmq_reactor_remove(preactor);
	int rc = zmq_close(preactor->socket);
	return rc;
}


//
// zmq_reactor_connect - sets new socket, returns old socket
//
void* zmq_reactor_connect(zmq_reactor_t* preactor, void* socket)
{
	assert(preactor != NULL);
	std::swap(socket, preactor->socket);
	return socket;
}


//
// zmq_reactor_socket - returns current socket
//
void* zmq_reactor_socket(zmq_reactor_t* preactor)
{
	assert(preactor != NULL);
	return preactor->socket;
}


//
// zmq_reactor_events
//
short zmq_reactor_events(zmq_reactor_t* preactor, short events)
{
	assert(preactor != NULL);
	std::swap(events, preactor->events);
	return events;
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
// callback_pollitems_t
//
typedef int callback_pollitems_t(zmq_pollitem_t* begin, zmq_pollitem_t* end, zmq_reactor_t** preactor);


//
// callback_pollitems (static)
//
static int callback_pollitems(zmq_pollitem_t* begin, zmq_pollitem_t* end, zmq_reactor_t** preactor)
{
	int rc = 0;
	for (; begin != end; ++begin, ++preactor) {
		if (begin->revents) {
			rc = ((*preactor)->handler)(begin->socket, begin->revents, *preactor, (*preactor)->hint);
			// force return if non-zero
			if (rc)
				break;
		}
	}
	return rc;
}

//
// callback_repollitems
//
static int callback_repollitems(zmq_pollitem_t* begin, zmq_pollitem_t* end, zmq_reactor_t** preactor)
{
	int rc = 0;
	if (begin != end) {
		for (;;) {
			if (begin->revents) {
				rc = ((*preactor)->handler)(begin->socket, begin->revents, *preactor, (*preactor)->hint);
				// force return if non-zero
				if (rc)
					break;
				
				// bump and break if at end
				++begin;
				++preactor;
				
				if (begin == end)
					break;
				
				// re-poll all elements from here to end (immediate return)
				rc = zmq_poll(begin, end - begin, 0);			
				if (rc == -1 || rc == 0)
					break;
			}
		}
	}
	return rc;
}

//
// MOVED HERE TEMPORARILY TO COMPILE
// zmq_reactor_policy_t - user supplied function to select reactor ordering
//
// policy can return one of the following:
//	- same reactor	= poll_list remains the same
//	- new reactor	= poll_list is rebuilt, and starts with new reactor
//	- NULL			= zmq_reactor_poll() gracefully returns with 0 (= no error)
//
typedef zmq_reactor_t* zmq_reactor_policy_t(zmq_reactor_t*, void* hint);


//
// zmq_reactor_repoll_policy
//
int zmq_reactor_framework(zmq_reactor_t* start, long timeout, callback_pollitems_t* callback, zmq_reactor_policy_t* policy, void* hint)
{
	// TODO: is std::vector efficient here?
	zmq_reactor_t* reactors[MAX_POLL];
	zmq_pollitem_t items[MAX_POLL];
	
	assert(start != NULL);
	
	// initialize pollitems
	zmq_pollitem_t* end_item = build_pollitems(start, items, reactors);
	
	for (;;) {
		int rc = 0;
		
		rc = zmq_poll(items, end_item - items, timeout);
		// rc == 0 if timeout reached, or if no items were enabled for polling
		if (rc == -1 || rc == 0)
			return rc;
		
		assert(rc > 0);
		
		// callback pollitems
		rc = (*callback)(items, end_item, reactors);
		if (rc)
			return rc;
		
		// handle NULL policy as "once only"
		if (policy == NULL)
			return 0;
		
		// else: execute policy
		zmq_reactor_t* next = (*policy)(*reactors, hint);
		
		// return, refresh, rebuild
		if (next == NULL)
			return 0;
		else if (next == *reactors)
			refresh_pollitems(items, end_item, reactors);
		else 
			end_item = build_pollitems(start, items, reactors);
	}
}


//
// zmq_reactor_poll_policy - poll using default policy
//
int zmq_reactor_poll_policy(zmq_reactor_t* preactor, long timeout, zmq_reactor_policy_t* policy, void* hint)
{
	return zmq_reactor_framework(preactor, timeout, callback_pollitems, policy, 0);
}


//
// default_policy (static)
//
static zmq_reactor_t* default_policy(zmq_reactor_t* reactor, void* hint)
{
	return reactor;
}

//
// zmq_reactor_poll_policy - poll using default policy
//
int zmq_reactor_repoll(zmq_reactor_t* preactor, long timeout)
{
	return zmq_reactor_framework(preactor, timeout, callback_repollitems, default_policy, 0);
}


//
// zmq_reactor_poll - poll using default policy
//
int zmq_reactor_poll(zmq_reactor_t* preactor, long timeout)
{
	return zmq_reactor_framework(preactor, timeout, callback_pollitems, default_policy, 0);
}
