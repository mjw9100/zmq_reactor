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

#include "zmq_reactor_uuid.h"

// one way to build a uuid -- 
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"

#include <string>
using namespace std;

// TODO: is it better to clone the identity code? must verify that nulls will not appear
string zmq_reactor_uuid()
{
	boost::uuids::random_generator gen;
	boost::uuids::uuid u = gen();
	const size_t uuid_len = sizeof u.data / sizeof u.data[0];
	string s;
	uint8_t base = 'A';
	for (uint8_t* pc = u.data; pc != u.data + uuid_len; ++pc) {
		if (*pc)
			s.push_back(*pc);
		else {
			s.push_back(base);
			++base;
		}
	}
	s.push_back(base);
	return s;
}
