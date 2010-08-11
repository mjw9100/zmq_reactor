/*
 * ----------------------------------------------------------
 *
 * Copyright 2010 M Weinstein
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

#include "zmq.h"
#include "zmq_reactor.h"
#include "zmq_reactor_ops.h"
#include "zmq_reactor_pair.h"

#include <cassert>
#include <iostream>

//
// timer_thread - called with connect for zmq_reactor_pair
//
void* timer_thread(void* connect)
{
	assert(connect != NULL);
	
	
	// connect to the other side of the pair
	void* socket = zmq_reactor_pair_connect(connect);
	assert(socket != NULL);
	
	zmq_msg_t msg;
	
	unsigned long cycle = 0;
	
	for (;;) {
		char buff[256];
		sprintf(buff, "SDRAWKCAB<<< %06ld", ++cycle);
		size_t len = strlen(buff);
		
		// build message
		int rc = zmq_msg_init_size(&msg, len);
		assert(rc == 0);
		
		// copy data
		memcpy(zmq_msg_data(&msg), buff, len);
		
		// send message
		rc = zmq_send(socket, &msg, 0);
		assert(rc == 0);
		
		// release message contents
		rc = zmq_msg_close(&msg);
		assert(rc == 0);
		
		// wait a bit
		usleep(100000);
	}
	
	return 0;
}

//
// reverse some bytes
//
void reverse(char* begin, char* end)
{
	// swap bytes
	while (begin < end) {
		char temp = *begin;
		*begin++ = *--end;
		*end = temp;
	}
}

//
// echo_thread - reverse received packets
//
void* echo_thread(void* connect)
{
	assert(connect != NULL);
	
	// connect to the other side of the pair
	void* socket = zmq_reactor_pair_connect(connect);
	assert(socket != NULL);
	
	zmq_msg_t msg;
	
	int rc = zmq_msg_init(&msg);
	assert(rc == 0);
	
	for (;;) {
		// get message
		rc = zmq_recv(socket, &msg, 0);
		assert(rc == 0);
		
		// reverse message
		assert(zmq_msg_size(&msg) != 0);
		char* begin = (char*)zmq_msg_data(&msg);
		char* end = begin + zmq_msg_size(&msg);
		reverse(begin, end);
		
		// send back
		rc = zmq_send(socket, &msg, 0);
		assert(rc == 0);
	}
	
	return 0;
}


//
// timer_handler - forward message to echo service
//
static int timer_handler(void* socket, short events, zmq_reactor_t* reactor, void* hint)
{
	zmq_msg_t msg;
	int rc = zmq_msg_init(&msg);
	assert(rc == 0);
	
	// receive tick from socket
	rc = zmq_recv(socket, &msg, 0);
	assert(rc == 0);
	
	// send to echo socket
	rc = zmq_send(hint, &msg, 0);
	assert(rc == 0);
	
	return 0;
}

//
// echo_handler - output received message
//

static int echo_handler(void* socket, short events, zmq_reactor_t* reactor, void* hint)
{
	zmq_msg_t msg;
	int rc = zmq_msg_init(&msg);
	assert(rc == 0);
	
	// receive message
	rc = zmq_recv(socket, &msg, 0);
	assert(rc == 0);
	
	// output message
	size_t bytes = fwrite(zmq_msg_data(&msg), zmq_msg_size(&msg), 1, stdout);
	assert(bytes != 0);
	printf("\n");
	
	assert(bytes != 0);
	
	return 0;
}


int main(int argc, char** argv)
{
	
	void* context = zmq_init(1);
	assert(context != NULL);

	// ECHO REACTOR
	zmq_reactor_t echo;

	int rc = zmq_reactor_init(&echo, ZMQ_POLLIN, echo_handler, 0);
	assert(rc == 0);
	
	// create echo pair
	void* cecho = zmq_reactor_pair(context, &echo);
	
	// TIMER REACTOR
	zmq_reactor_t timer;
	
	// wire echo pair to timer reactor
	rc = zmq_reactor_init(&timer, ZMQ_POLLIN, timer_handler, zmq_reactor_socket(&echo));
	assert(rc == 0);

	// create timer pair
	void* ctimer = zmq_reactor_pair(context, &timer);
	
	// assemble queue
	zmq_reactor_insert(&timer, &echo);
	
	// start threads
	pthread_t worker;
	rc = pthread_create(&worker, NULL, timer_thread, ctimer);
	assert (rc == 0);
	
	rc = pthread_create(&worker, NULL, echo_thread, cecho);
	assert (rc == 0);
	
	// poll
	rc = zmq_reactor_poll(&timer, -1);
	return rc;
}
