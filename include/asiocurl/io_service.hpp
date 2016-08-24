/**
 *	\file
 */


#pragma once


#include "future.hpp"
#include <boost/asio.hpp>
#include <curl/curl.h>
#include <cstddef>
#include <exception>
#include <memory>
#include <mutex>
#include <unordered_map>


namespace asiocurl {


	/**
	 *	Services curl easy handles using Boost.ASIO.
	 */
	class io_service {


		public:


			/**
			 *	The type of a curl multi handle.
			 */
			using native_handle_type=CURLM *;


		private:


			class control {


				private:


					using mutex_type=std::recursive_mutex;
					mutable mutex_type m_;
					bool stop_;


				public:


					control (const control &) = delete;
					control (control &&) = delete;
					control & operator = (const control &) = delete;
					control & operator = (control &&) = delete;


					control ();


					using guard_type=std::unique_lock<mutex_type>;
					guard_type lock () const noexcept;
					explicit operator bool () const noexcept;
					void stop () noexcept;


			};


			class easy_state {


				public:


					CURL * easy;
					std::exception_ptr ex;
					promise<CURLMsg> promise;


					easy_state () = delete;
					easy_state (const easy_state &) = delete;
					easy_state (easy_state &&) = default;
					easy_state & operator = (const easy_state &) = delete;
					easy_state & operator = (easy_state &&) = delete;


					explicit easy_state (CURL *);


					void set_exception () noexcept;
					void set_exception (std::exception_ptr) noexcept;


			};


			class socket_state {


				public:


					int what;
					bool read;
					bool write;
					bool closed;
					boost::asio::ip::tcp::socket socket;


					socket_state () = delete;
					socket_state (const socket_state &) = delete;
					socket_state (socket_state &&) = default;
					socket_state & operator = (const socket_state &) = delete;
					socket_state & operator = (socket_state &&) = delete;


					socket_state (const boost::asio::ip::tcp::socket::protocol_type &, boost::asio::io_service &);


			};


			boost::asio::io_service & ios_;
			native_handle_type handle_;
			using handles_type=std::unordered_map<CURL *,easy_state>;
			handles_type handles_;
			using sockets_type=std::unordered_map<curl_socket_t,socket_state>;
			sockets_type sockets_;
			std::shared_ptr<control> control_;
			boost::asio::deadline_timer timer_;


			static curl_socket_t open (void *, curlsocktype, struct curl_sockaddr *) noexcept;
			static int close (void *, curl_socket_t) noexcept;
			static int socket (CURL *, curl_socket_t, int, void *, void *) noexcept;
			static int timer (CURLM *, long, void *) noexcept;


			void do_action (curl_socket_t, int) noexcept;
			void remove_socket (sockets_type::iterator) noexcept;
			void abort (easy_state &) noexcept;
			void abort (handles_type::iterator) noexcept;
			void complete (CURLMsg) noexcept;
			void read (socket_state &);
			void write (socket_state &);


		public:


			io_service () = delete;
			io_service (const io_service &) = delete;
			io_service (io_service &&) = delete;
			io_service & operator = (const io_service &) = delete;
			io_service & operator = (io_service &&) = delete;


			/**
			 *	Creates a new io_service which uses a certain
			 *	boost::asio::io_service for socket I/O and timeouts.
			 *
			 *	\param [in] ios
			 *		The boost::asio::io_service with which this io_service
			 *		shall be associated.  This reference must remain valid
			 *		for the lifetime of the io_service or the behaviour is
			 *		undefined.
			 */
			explicit io_service (boost::asio::io_service & ios);


			/**
			 *	Shuts the io_service down.
			 *
			 *	Any pending transfers will be immediately aborted.
			 *
			 *	Any pending asynchronous operations will be cancelled.  When
			 *	those pending operations complete they will be no ops (i.e.
			 *	it is not necessary to drain the boost::asio::io_service before
			 *	calling this destructor).
			 */
			~io_service () noexcept;


			/**
			 *	Adds a curl easy handle to be managed by this io_service.
			 *
			 *	The io_service does not assume ownership of the easy handle
			 *	and the easy handle must remain valid until the io_service
			 *	is done with it or the behaviour is undefined.
			 *
			 *	Once the returned future completes the io_service no longer
			 *	requires the easy handle's lifetime to persist.  At this point
			 *	only four things may be safely done with the easy handle:
			 *
			 *	-	curl_easy_reset
			 *	-	curl_easy_cleanup
			 *	-	curl_easy_getinfo
			 *	-	A series of curl API calls with the purpose of reusing
			 *		the easy handle where the new transfer is performed by
			 *		adding the handle back to this same io_service
			 *
			 *	Doing anything else with the easy handle at this point leads to
			 *	undefined behaviour.
			 *
			 *	\param [in] easy
			 *		The easy handle to add to the io_service.
			 *
			 *	\return
			 *		A handle to the future value of the completed transfer
			 *		represented by the easy handle.
			 */
			future<CURLMsg> add (CURL * easy);


			/**
			 *	Removes a curl easy handle from the io_service.
			 *
			 *	Once this call completes the io_service will no longer
			 *	use the easy handle.
			 *
			 *	Note that the io_service does not reset any options it has
			 *	set on the easy handle.  Accordingly reusing the easy handle
			 *	without calling curl_easy_reset on it leads to undefined behaviour
			 *	unless the new transfer is handled through the same io_service.
			 *
			 *	If \em easy was never added to this io_service, or if the
			 *	transfer has completed, nothing happens.
			 *
			 *	Note that it is not necessary to call this function after
			 *	a transfer completes: In this instance the easy handle is
			 *	automatically disassociated from the io_service.
			 *
			 *	\param [in] easy
			 *		The easy handle to disassociate from the io_service.
			 *
			 *	\return
			 *		\em true if \em easy was disassociated from the
			 *		io_service, \em false if \em easy was not associated
			 *		with the io_service.
			 */
			bool remove (CURL * easy) noexcept;


			/**
			 *	Returns the boost::asio::io_service associated with this
			 *	object.
			 *
			 *	This method is named get_io_service rather than io_service
			 *	(which would be in keeping with standard library naming
			 *	conventions) for consistency with other classes in Boost.ASIO
			 *	which use the name get_io_service.
			 *
			 *	\return
			 *		A reference to the associated boost::asio::io_service.
			 */
			boost::asio::io_service & get_io_service () const noexcept;


			/**
			 *	Retrieves the curl multi handle this io_service object
			 *	wraps.
			 *
			 *	\return
			 *		A curl multi handle.
			 */
			native_handle_type native_handle () const noexcept;


	};


}
