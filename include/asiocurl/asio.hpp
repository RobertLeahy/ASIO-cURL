/**
 *	\file
 */


#pragma once


#include "configure.hpp"


#ifdef ASIOCURL_USE_BOOST_ASIO
#ifndef BOOST_ASIO_HAS_STD_CHRONO
#define BOOST_ASIO_HAS_STD_CHRONO
#endif
#include <boost/asio.hpp>
//	I don't know why this is necessary
#include <boost/asio/steady_timer.hpp>
#else
#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif
#ifndef ASIO_HAS_STD_CHRONO
#define ASIO_HAS_STD_CHRONO
#endif
#ifndef ASIO_HAS_THREADS
#define ASIO_HAS_THREADS
#endif
//	We have to include Windows.h otherwise
//	Windows version macros that ASIO depends on
//	are missing
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif
#include <asio.hpp>
#endif


namespace asiocurl {


	namespace asio {


		#ifdef ASIOCURL_USE_BOOST_ASIO
		using namespace boost::asio;
		#else
		using namespace ::asio;
		#endif


	}


}
