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


#include "zmq_reactor_ops.h"

#include <cassert>

//
// zmq_reactor_ops_normal - supplied for completeness
//
void zmq_reactor_ops_normal(zmq_reactor_t* start)
{
	assert(start != NULL);
	
	// always process at least one
	for (zmq_reactor_t* pcur = start; ; pcur = pcur->next) {
		pcur->ops = ZMQ_REACTOR_OPS_NOP;
		// break
		if (pcur->next == start)
			break;
	}
}

//
// zmq_reactor_ops_priority - priority polling
//
void zmq_reactor_ops_priority(zmq_reactor_t* start)
{
	assert(start != NULL);
	
	// always process at least one
	for (zmq_reactor_t* pcur = start; ; pcur = pcur->next) {
		pcur->ops = ZMQ_REACTOR_OPS_POLA;
		// break
		if (pcur->next == start)
			break;
	}
}

//
// zmq_reactor_ops_priority - priority polling
//
void zmq_reactor_ops_trailing(zmq_reactor_t* start)
{
	assert(start != NULL);
	
	// always process at least one
	for (zmq_reactor_t* pcur = start; ; pcur = pcur->next) {
		if (pcur->next != start)
			pcur->ops = ZMQ_REACTOR_OPS_POLFI + 1;
		else {
			pcur->ops = ZMQ_REACTOR_OPS_NOP;
			break;
		}
	}
}


