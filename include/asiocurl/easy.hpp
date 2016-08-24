/**
 *	\file
 */


#pragma once


#include <curl/curl.h>


namespace asiocurl {


	/**
	 *	A basic RAII wrapper for a curl easy handle.
	 */
	class easy {


		public:


			/**
			 *	The type of a curl easy handle.
			 */
			using native_handle_type=CURL *;


		private:


			native_handle_type handle_;


			void destroy () noexcept;


		public:


			/**
			 *	Creates an easy object which wraps a newly-created
			 *	curl easy handle.
			 */
			easy ();
			/**
			 *	Creates an easy object which assumes ownership
			 *	of a curl easy handle.
			 *
			 *	\param [in] easy
			 *		A curl easy handle.
			 */
			explicit easy (native_handle_type easy) noexcept;
			/** 
			 *	Creates an easy object by duplicating another
			 *	easy handle.
			 *
			 *	This constructor invokes curl_easy_duphandle
			 *	internally and that therefore all restrictions on
			 *	the invocation of that function apply to this
			 *	constructor.
			 *
			 *	\param [in] rhs
			 *		The easy object whose managed easy handle shall
			 *		be duplicated.
			 */
			easy (const easy & rhs);
			/**
			 *	Creates an easy object by taking ownership
			 *	of the easy handle managed by another easy object.
			 *
			 *	After this constructor executes the only things which
			 *	may be safely done with \em rhs are:
			 *
			 *	-	Destroy it
			 *	-	Assign to it
			 *
			 *	\param [in] rhs
			 *		The easy object whose easy handle the newly-created
			 *		object shall assume ownership of.
			 */
			easy (easy && rhs) noexcept;


			/**
			 *	Replaces the easy handle managed by this object with
			 *	a duplicate of the easy handle managed by another object.
			 *
			 *	This operator invokes curl_easy_duphandle internally and
			 *	therefore all restrictions on the invocation of that function
			 *	apply to this operator.
			 *
			 *	\param [in] rhs
			 *		The easy object whose managed easy handle shall be
			 *		duplicated.
			 *
			 *	\return
			 *		A reference to this object.
			 */
			easy & operator = (const easy & rhs);
			/**
			 *	Replaces the easy handle managed by this object with
			 *	the easy handle managed by another object.
			 *
			 *	After this operator executes the only things which may
			 *	be safely done with \em rhs are.
			 *
			 *	-	Destroy it
			 *	-	Assign to it
			 *
			 *	\param [in] rhs
			 *		The easy object whose easy handle this object shall
			 *		assume ownership of.
			 *
			 *	\return
			 *		A reference to this object.
			 */
			easy & operator = (easy && rhs) noexcept;


			/**
			 *	Cleans up the managed easy handle.
			 */
			~easy () noexcept;


			/**
			 *	Retrieves the wrapped curl easy handle.
			 *
			 *	\return
			 *		A curl easy handle.
			 */
			native_handle_type native_handle () const noexcept;
			/**
			 *	Retrieves the wrapped curl easy handle, allowing
			 *	this object to be used as a drop-in replacement for
			 *	a raw curl easy handle.
			 *
			 *	\return
			 *		A curl easy handle.
			 */
			operator native_handle_type () const noexcept;


	};


	/**
	 *	An RAII wrapper for a curl easy handle which also
	 *	sets CURLOPT_ERRORBUFFER thereupon and maintains
	 *	the error buffer.
	 */
	class easy_with_error_buffer : public easy {


		private:


			char buffer_ [CURL_ERROR_SIZE];


			void set ();


		public:


			easy_with_error_buffer (const easy_with_error_buffer &) = delete;
			easy_with_error_buffer (easy_with_error_buffer &&) = delete;
			easy_with_error_buffer & operator = (const easy_with_error_buffer &) = delete;
			easy_with_error_buffer & operator = (easy_with_error_buffer &&) = delete;


			/**
			 *	Creates a new easy handle and installs the
			 *	internal buffer.
			 */
			easy_with_error_buffer ();
			/**
			 *	Assumes ownership of an easy handle and installs
			 *	the internal buffer.
			 *
			 *	\param [in] easy
			 *		The easy handle to assume ownership of.
			 */
			explicit easy_with_error_buffer (native_handle_type easy);


			/**
			 *	Clears the stored error message, if any.
			 */
			void clear () noexcept;

			
			/**
			 *	Retrieves the stored error message, if any.
			 *
			 *	\return
			 *		The stored error message as a C-style string.
			 *		This pointer remains valid only so long as the
			 *		lifetime of this object persists.
			 */
			const char * what () const noexcept;


			/**
			 *	Determines whether this object contains an error
			 *	message.
			 *
			 *	\return
			 *		\em false if this object contains an error
			 *		message, \em true otherwise.
			 */
			bool empty () const noexcept;


			/**
			 *	Throws a \ref easy_error which encapsulates \em code
			 *	and whose error message is:
			 *
			 *	-	The contents of this object's internal buffer, if
			 *		any
			 *	-	Otherwise the result of calling curl_easy_strerror
			 *		on \em code
			 *
			 *	This object's internal buffer will be cleared before
			 *	this function returns.
			 *
			 *	\param [in] code
			 *		A CURLcode describing the error which occurred.
			 */
			[[noreturn]]
			void raise (CURLcode code);


	};


}
