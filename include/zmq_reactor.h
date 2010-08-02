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
	short					options;
	short					events;
	void*					socket;
	zmq_reactor_handler_t*	handler;
	void*					hint;
};

//
// reactor callback options
//
// NB microcode-like reactor options -- helper functions TBD in zmq_reactor_options.h
//
// ZMQ_REACTOR_OPT_ALWAYS - always callback even if no event occurred
//
// ZMQ_REACTOR_OPT_EXIT - always callback even if no event occurred
//
// ZMQ_REACTOR_OPT_RESTART - after calling, restart poll cycle
// branch offset used to select next starting point (offset from 0)
//
// ZMQ_REACTOR_OPT_RELATIVE - offset on restart is relative to this reactor
// NB this may not be final
//
// ZMQ_REACTOR_OPT_OFFSET_MASK - offset [0..15] to next reactor
// executes relative branch to next reactor if non-zero
//

#define	ZMQ_REACTOR_OPT_ALWAYS		0x0080
#define	ZMQ_REACTOR_OPT_EXIT		0x0040
#define	ZMQ_REACTOR_OPT_RESTART		0x0020
#define	ZMQ_REACTOR_OPT_RELATIVE	0x0010
#define	ZMQ_REACTOR_OPT_OFFSET_MASK	0x000f

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
// zmq_reactor_enable - sets options for reactor
//
// returns: previous options
//
short zmq_reactor_options(zmq_reactor_t*, short options);


// 
// zmq_reactor_poll - poll reactors using the default polling policy (zmq_reactor_policy_static)
//
int zmq_reactor_poll(zmq_reactor_t*, long timeout); 

#endif//__ZMQ_REACTOR_H_INCLUDED__