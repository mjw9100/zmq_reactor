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

#ifndef	__ZMQ_REACTOR_OPS_H_INCLUDED__
#define	__ZMQ_REACTOR_OPS_H_INCLUDED__

#include "zmq_reactor.h"

//
// zmq_reactor_ops_normal - supplied for completeness
//
// Pattern: standard in-order polling
// 
void zmq_reactor_ops_normal(zmq_reactor_t*, int nitems);

//
// zmq_reactor_ops_priority - priority polling
//
// Pattern: priority polling - initiates a new poll from the start after every reactor call
// 
void zmq_reactor_ops_priority(zmq_reactor_t*, int nitems);

//
// zmq_reactor_ops_trailing - poll trailing sockets
//
// Pattern: trailing poll - sockets following are polled again
// 
void zmq_reactor_ops_trailing(zmq_reactor_t*, int nitems);

#endif//__ZMQ_REACTOR_OPS_H_INCLUDED__
