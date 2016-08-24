#include <asiocurl/exception.hpp>
#include <asiocurl/io_service.hpp>
#include <asiocurl/scope.hpp>
#include <boost/asio.hpp>
#include <curl/curl.h>
#include <exception>
#include <memory>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <utility>


namespace asiocurl {


	static void multi_check (CURLMcode code) {

		if (code!=CURLM_OK) throw multi_error(code);

	}


	static void easy_check (CURLcode code) {

		if (code!=CURLE_OK) throw easy_error(code);

	}


	io_service::control::control () : stop_(false) {	}


	io_service::control::guard_type io_service::control::lock () const noexcept {

		return guard_type(m_);

	}


	io_service::control::operator bool () const noexcept {

		return !stop_;

	}


	void io_service::control::stop () noexcept {

		stop_=true;

	}


	io_service::easy_state::easy_state (CURL * easy) : easy(easy) {	}


	void io_service::easy_state::set_exception () noexcept {

		set_exception(std::current_exception());

	}


	void io_service::easy_state::set_exception (std::exception_ptr ptr) noexcept {

		if (!ex) ex=std::move(ptr);

	}


	io_service::socket_state::socket_state (const boost::asio::ip::tcp::socket::protocol_type & protocol, boost::asio::io_service & ios)
		:	what(CURL_POLL_NONE),
			read(false),
			write(false),
			closed(false),
			socket(ios)
	{

		socket.open(protocol);

	}


	curl_socket_t io_service::open (void * clientp, curlsocktype purpose, struct curl_sockaddr * address) noexcept {

		if (purpose!=CURLSOCKTYPE_IPCXN) return CURL_SOCKET_BAD;
		if ((address->family!=AF_INET) && (address->family!=AF_INET6)) return CURL_SOCKET_BAD;

		auto & self=*static_cast<io_service *>(clientp);

		try {

			socket_state ss((address->family==AF_INET) ? boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6(),self.ios_);
			auto native_handle=ss.socket.native_handle();
			self.sockets_.emplace(native_handle,std::move(ss));

			return native_handle;

		} catch (...) {	}

		return CURL_SOCKET_BAD;

	}


	int io_service::close (void * clientp, curl_socket_t item) noexcept {

		auto & self=*static_cast<io_service *>(clientp);

		//	This should always be found and should never
		//	throw
		auto iter=self.sockets_.find(item);
		auto & ss=iter->second;
		ss.closed=true;

		int retr=0;
		try {

			ss.socket.close();

		} catch (...) {

			retr=1;

		}

		self.remove_socket(iter);

		return retr;

	}


	static bool is_read (int what) noexcept {

		switch (what) {

			default:
				break;
			case CURL_POLL_IN:
			case CURL_POLL_INOUT:
				return true;

		}

		return false;

	}


	static bool is_write (int what) noexcept {

		switch (what) {

			default:
				break;
			case CURL_POLL_OUT:
			case CURL_POLL_INOUT:
				return true;

		}

		return false;

	}


	int io_service::socket (CURL * easy, curl_socket_t socket, int what, void * userp, void *) noexcept {

		auto & self=*static_cast<io_service *>(userp);
		auto & s=self.handles_.find(easy)->second;
		auto & ss=self.sockets_.find(socket)->second;
		ss.what=what;

		try {

			if (is_read(what) && !ss.read) self.read(ss);

			if (is_write(what) && !ss.write) self.write(ss);

			return 0;

		} catch (...) {

			s.set_exception();

		}

		return -1;

	}


	int io_service::timer (CURLM *, long timeout_ms, void * userp) noexcept {

		auto & self=*static_cast<io_service *>(userp);

		try {

			self.timer_.cancel();

			if (timeout_ms<=0) {

				self.do_action(CURL_SOCKET_TIMEOUT,0);
				return 0;

			}

			self.timer_.expires_from_now(boost::posix_time::millisec(timeout_ms));
			self.timer_.async_wait([&self,control=self.control_] (const auto &) {

				auto l=control->lock();
				if (!*control) return;
				self.do_action(CURL_SOCKET_TIMEOUT,0);

			});

			return 0;

		} catch (...) {	}

		return -1;

	}


	void io_service::do_action (curl_socket_t socket, int mask) {

		for (;;) {

			int ignored;
			auto result=curl_multi_socket_action(handle_,socket,mask,&ignored);
			if (result==CURLM_CALL_MULTI_PERFORM) continue;
			if (result==CURLM_OK) break;
			throw multi_error(result);

		}

		for (;;) {

			int ignored;
			auto ptr=curl_multi_info_read(handle_,&ignored);
			if (ptr==nullptr) break;
			if (ptr->msg!=CURLMSG_DONE) continue;
			complete(*ptr);

		}

	}


	void io_service::remove_socket (sockets_type::iterator iter) noexcept {

		auto & ss=iter->second;

		//	Still waiting on a callback to complete
		if (ss.read) return;
		if (ss.write) return;
		//	Still waiting on libcurl to close the socket
		if (!ss.closed) return;

		sockets_.erase(iter);

	}


	void io_service::abort (easy_state & s) noexcept {

		std::exception_ptr ex=s.ex;
		if (!ex) try {

			ex=std::make_exception_ptr(aborted{});

		} catch (...) {

			ex=std::current_exception();

		}

		s.promise.set_exception(std::move(ex));

		//	This may throw into noexcept, it should never happen
		//	as far as I'm concerned, but it's better to fail
		//	fast and in the correct place when/if it does
		multi_check(curl_multi_remove_handle(handle_,s.easy));

	}


	void io_service::abort (handles_type::iterator iter) noexcept {

		abort(iter->second);

		//	This will cause the easy handle to be cleaned up,
		//	which should cause all sockets to be closed, this will
		//	prevent any pending asynchronous callbacks from accessing
		//	the easy handle
		handles_.erase(iter);

	}


	void io_service::complete (CURLMsg msg) noexcept {

		auto iter=handles_.find(msg.easy_handle);
		auto & s=iter->second;
		if (s.ex) s.promise.set_exception(std::move(s.ex));
		else s.promise.set_value(msg);
		handles_.erase(iter);

	}


	void io_service::read (socket_state & ss) {

		ss.socket.async_read_some(boost::asio::null_buffers{},[&,control=control_] (const auto & ec, auto) {

			auto l=control->lock();
			if (!*control) return;
			ss.read=false;
			auto handle=ss.socket.native_handle();
			if (ss.closed) {

				this->remove_socket(this->sockets_.find(handle));
				return;

			}

			int mask=CURL_CSELECT_IN;
			if (ec) mask|=CURL_CSELECT_ERR;
			this->do_action(handle,mask);

		});
		ss.read=true;

	}


	void io_service::write (socket_state & ss) {

		ss.socket.async_write_some(boost::asio::null_buffers{},[&,control=control_] (const auto & ec, auto) {

			auto l=control->lock();
			if (!*control) return;
			ss.write=false;
			auto handle=ss.socket.native_handle();
			if (ss.closed) {

				this->remove_socket(this->sockets_.find(handle));
				return;

			}

			int mask=CURL_CSELECT_OUT;
			if (ec) mask|=CURL_CSELECT_ERR;
			this->do_action(handle,mask);

		});
		ss.write=true;

	}


	io_service::io_service (boost::asio::io_service & ios) : ios_(ios), control_(std::make_shared<control>()), timer_(ios) {

		if (!(handle_=curl_multi_init())) throw error("curl_multi_init failed");
		auto g=make_scope_exit([&] () noexcept {	multi_check(curl_multi_cleanup(handle_));	});

		//	Install our handlers
		multi_check(curl_multi_setopt(handle_,CURLMOPT_SOCKETFUNCTION,&socket));
		multi_check(curl_multi_setopt(handle_,CURLMOPT_SOCKETDATA,this));
		multi_check(curl_multi_setopt(handle_,CURLMOPT_TIMERFUNCTION,&timer));
		multi_check(curl_multi_setopt(handle_,CURLMOPT_TIMERDATA,this));

		g.release();

	}


	io_service::~io_service () noexcept {

		auto l=control_->lock();
		//	Destroy all the easy handles to abort all
		//	transfers
		for (auto && pair : handles_) abort(pair.second);
		handles_.clear();

		//	Make sure callbacks abort as soon as they're
		//	fired
		control_->stop();

		//	This should never throw, but if it does
		//	it's better to fail fast than to silently
		//	continue
		multi_check(curl_multi_cleanup(handle_));

	}


	future<CURLMsg> io_service::add (CURL * easy) {

		auto l=control_->lock();

		auto pair=handles_.emplace(std::piecewise_construct,std::forward_as_tuple(easy),std::forward_as_tuple(easy));
		auto iter=pair.first;
		auto g=make_scope_exit([&] () noexcept {	handles_.erase(iter);	});
		if (!pair.second) throw std::logic_error("Attempt to add duplicate easy handle");
		auto & s=iter->second;
		auto retr=s.promise.get_future();

		easy_check(curl_easy_setopt(s.easy,CURLOPT_OPENSOCKETFUNCTION,&open));
		easy_check(curl_easy_setopt(s.easy,CURLOPT_OPENSOCKETDATA,this));
		easy_check(curl_easy_setopt(s.easy,CURLOPT_CLOSESOCKETFUNCTION,&close));
		easy_check(curl_easy_setopt(s.easy,CURLOPT_CLOSESOCKETDATA,this));

		//	This should invoke the proper callbacks to get things
		//	rolling
		multi_check(curl_multi_add_handle(handle_,easy));

		g.release();

		return retr;

	}


	bool io_service::remove (CURL * easy) noexcept {

		auto l=control_->lock();

		auto iter=handles_.find(easy);
		if (iter==handles_.end()) return false;

		abort(iter);
		return true;

	}


	boost::asio::io_service & io_service::get_io_service () const noexcept {

		return ios_;

	}


	io_service::native_handle_type io_service::native_handle () const noexcept {

		return handle_;

	}


}
