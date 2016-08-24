#include <asiocurl/easy.hpp>
#include <asiocurl/exception.hpp>
#include <asiocurl/scope.hpp>
#include <curl/curl.h>
#include <utility>


namespace asiocurl {


	void easy::destroy () noexcept {

		if (!handle_) return;
		curl_easy_cleanup(handle_);
		handle_=nullptr;

	}


	static easy::native_handle_type duphandle (easy::native_handle_type handle) {

		if (!handle) return nullptr;
		auto retr=curl_easy_duphandle(handle);
		if (!retr) throw error("curl_easy_duphandle failed");
		return retr;

	}


	easy::easy () : handle_(curl_easy_init()) {

		if (!handle_) throw error("curl_easy_init failed");

	}


	easy::easy (native_handle_type easy) noexcept : handle_(easy) {	}


	easy::easy (const easy & rhs) : handle_(duphandle(rhs.handle_)) {	}


	easy::easy (easy && rhs) noexcept : handle_(rhs.handle_) {

		rhs.handle_=nullptr;

	}


	easy & easy::operator = (const easy & rhs) {

		if (&rhs!=this) {

			destroy();

			handle_=duphandle(rhs.handle_);

		}

		return *this;

	}


	easy & easy::operator = (easy && rhs) noexcept {

		destroy();

		using std::swap;
		swap(handle_,rhs.handle_);

		return *this;

	}


	easy::~easy () noexcept {

		destroy();

	}


	easy::native_handle_type easy::native_handle () const noexcept {

		return handle_;

	}


	easy::operator native_handle_type () const noexcept {

		return handle_;

	}


	void easy_with_error_buffer::set () {

		char * ptr=buffer_;
		auto result=curl_easy_setopt(native_handle(),CURLOPT_ERRORBUFFER,ptr);
		if (result!=CURLE_OK) throw easy_error(result);

	}


	easy_with_error_buffer::easy_with_error_buffer () {

		clear();
		set();

	}


	easy_with_error_buffer::easy_with_error_buffer (native_handle_type e) : easy(e) {

		clear();
		set();

	}


	void easy_with_error_buffer::clear () noexcept {

		buffer_[0]='\0';

	}


	const char * easy_with_error_buffer::what () const noexcept {

		return buffer_;

	}


	bool easy_with_error_buffer::empty () const noexcept {

		return buffer_[0]=='\0';

	}


	[[noreturn]]
	void easy_with_error_buffer::raise (CURLcode code) {

		auto g=make_scope_exit([&] () noexcept {	clear();	});
		if (empty()) throw easy_error(code);
		throw easy_error(code,buffer_);

	}


}
