#include <asiocurl/exception.hpp>
#include <curl/curl.h>


namespace asiocurl {


	aborted::aborted () : error("Transfer aborted") {	}


	multi_error::multi_error (CURLMcode code) : error(curl_multi_strerror(code)), code_(code) {	}


	CURLMcode multi_error::code () const noexcept {

		return code_;

	}


	easy_error::easy_error (CURLcode code) : error(curl_easy_strerror(code)), code_(code) {	}


	easy_error::easy_error (CURLcode code, const std::string & msg) : error(msg), code_(code) {	}


	CURLcode easy_error::code () const noexcept {

		return code_;

	}


}
