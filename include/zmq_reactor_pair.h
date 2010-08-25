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

#ifndef __ZMQ_REACTOR_PAIR_H_INCLUDED__
#define	__ZMQ_REACTOR_PAIR_H_INCLUDED__
#include "zmq_reactor.h"

//
// zmq_reactor_pair - creates unique inproc ZMQ_PAIR
//
// zmq_reactor_pair sets the socket of the supplied reactor, and returns a 
// "connect block" that can be used by zmq_reactor_pair_connect in a different
// thread to connect to the pair
//
// returns a "connector" usable by zmq_reactor_pair_connect, or NULL on failure
//
void* zmq_reactor_pair(void* context, zmq_reactor_t*);

//
// zmq_reactor_pair_connect - connect to the other side of the pair
//
// zmq_reactor_pair_connect converts a connector returned by zmq_reactor_pair to
// a socket.
//
// returns a socket pointer, or NULL on failure
//
// NB if called on the same connector twice, will fault or assert (block is freed
// after first call
//
void* zmq_reactor_pair_connect(void* connector);


#endif//__ZMQ_REACTOR_PAIR_H_INCLUDED__
