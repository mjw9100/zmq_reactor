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

#include "zmq.hpp"

#include "client.hpp"
#include "server.hpp"

#include <iostream>
#include <stdexcept>
using namespace std;

#include "queue_device.hpp"

int main(int argc, char** argv)
{

	zmq::context_t ctx(1);

	zmq::socket_t workers(ctx, ZMQ_XREQ);
	workers.bind("inproc://workers");
	
	zmq::socket_t clients(ctx, ZMQ_XREP);
	clients.bind("inproc://server");
	
	cout << "start_server" << endl;
	start_server(&ctx, 5);
	
	cout << "start_client" << endl;
	start_client(&ctx, 7);
	
	int rc = queue_device(clients, workers);
	return rc;
}
