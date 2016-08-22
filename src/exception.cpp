#include <asiocurl/exception.hpp>
#include <curl/curl.h>


namespace asiocurl {


	multi_error::multi_error (CURLMcode code) : error(curl_multi_strerror(code)), code_(code) {	}


	CURLMcode multi_error::code () const noexcept {

		return code_;

	}


}
