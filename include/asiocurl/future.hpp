/**
 *	\file
 */


#ifdef ASIOCURL_USE_BOOST_FUTURE
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
