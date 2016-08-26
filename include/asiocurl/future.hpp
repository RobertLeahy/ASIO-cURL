/**
 *	\file
 */


#pragma once


#include "configure.hpp"


#ifdef ASIOCURL_USE_BOOST_FUTURE
#ifndef BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE
#endif
#include <boost/thread/future.hpp>
#else
#include <future>
#endif


namespace asiocurl {


	#ifdef ASIOCURL_USE_BOOST_FUTURE
	using boost::future;
	using boost::promise;
	#else
	using std::future;
	using std::promise;
	#endif


}
