#include <asiocurl/exception.hpp>
#include <asiocurl/init.hpp>
#include <curl/curl.h>


namespace asiocurl {


	init::init () : init(CURL_GLOBAL_ALL) {	}


	init::init (long flags) {

		auto result=curl_global_init(flags);
		if (result!=CURLE_OK) throw easy_error(result);

	}


	init::~init () noexcept {

		curl_global_cleanup();

	}


}
