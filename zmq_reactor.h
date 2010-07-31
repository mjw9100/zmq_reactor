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


#ifndef	__ZMQ_REACTOR_H_INCLUDED__
#define	__ZMQ_REACTOR_H_INCLUDED__

//
// Polled "Reactor" prototype for zeromq
//
// Reactors may be linked to support aggregation of communication patterns
//

//
// zmq_reactor_handler_t - reactor callback prototype
//
//
// handler return values may be one of the following:
// zero		= normal processing
// non-zero	= zmq_reactor_poll_* return immediately with the supplied value
//
typedef int zmq_reactor_handler_t(void* socket, short revents, struct zmq_reactor_t*, void* hint);

// 
// zmq_reactor_t - contains handler, socket data
//
struct zmq_reactor_t {
	zmq_reactor_t*			next;
	zmq_reactor_t*			prev;
	short					events;
	void*					socket;
	zmq_reactor_handler_t*	handler;
	void*					hint;
};

//
// zmq_reactor_init - initialize reactor
//
int zmq_reactor_init(zmq_reactor_t*, short events, zmq_reactor_handler_t*, void* hint);

//
// zmq_reactor_init_socket - initialize reactor with specific zmq socket
//
int zmq_reactor_init_socket(zmq_reactor_t* reactor, void* socket, short events, zmq_reactor_handler_t* handler, void* hint);

// 
// zmq_reactor_insert - insert a reactor into a poll list
//
void zmq_reactor_insert(zmq_reactor_t* where, zmq_reactor_t*);

//
// zmq_reactor_remove - removes (unlinks) a reactor from a poll list
//
void zmq_reactor_remove(zmq_reactor_t*);

//
// zmq_reactor_close - removes reactor from a poll list and closes associated zmq socket (if any)
//
int zmq_reactor_close(zmq_reactor_t*);

//
// zmq_reactor_connect - sets socket for a reactor
//
// returns: previous zmq socket (NULL if unset)
//
void* zmq_reactor_connect(zmq_reactor_t*, void* socket);

//
// zmq_reactor_socket - returns the socket associated with a reactor
//
void* zmq_reactor_socket(zmq_reactor_t*);

//
// zmq_reactor_enable - enables/disables reactor
//
// returns: previous events
//
short zmq_reactor_events(zmq_reactor_t*, short events);

//
// zmq_reactor_policy_t - user supplied function to select reactor ordering
//
// policy can return one of the following:
//	- same reactor	= poll_list remains the same
//	- new reactor	= poll_list is rebuilt, and starts with new reactor
//	- NULL			= zmq_reactor_poll() gracefully returns with 0 (= no error)
//
typedef zmq_reactor_t* zmq_reactor_policy_t(zmq_reactor_t*, void* hint);

// 
// zmq_reactor_poll - poll reactors using the default polling policy (zmq_reactor_policy_static)
//
int zmq_reactor_poll(zmq_reactor_t*, long timeout); 

// 
// zmq_reactor_repoll - polls again after each upcall to pick up downstream changes
//
int zmq_reactor_repoll(zmq_reactor_t*, long timeout);

//
// zmq_reactor_poll_policy - poll reactors using a specific zmq_reactor_policy (see above)
//
// NB a policy value of NULL returns after one polling cycle
//
int zmq_reactor_poll_policy(zmq_reactor_t*, long timeout, zmq_reactor_policy_t*, void* hint); 


#endif//__ZMQ_REACTOR_H_INCLUDED__