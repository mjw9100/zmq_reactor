#ifndef	__SERVER_HPP__
#define	__SERVER_HPP__

#include "zmq.hpp"

void start_server(zmq::context_t* pctx, unsigned n);

#endif//__SERVER_HPP__