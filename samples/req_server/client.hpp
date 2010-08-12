#ifndef	__CLIENT_HPP__
#define	__CLIENT_HPP__

#include "zmq.hpp"

void start_client(zmq::context_t* pctx, unsigned n);

#endif//__CLIENT_HPP__