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