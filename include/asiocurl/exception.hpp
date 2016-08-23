/**
 *	\file
 */


#pragma once


#include <curl/curl.h>
#include <stdexcept>


namespace asiocurl {


	/**
	 *	A base class for all asiocurl exceptions.
	 */
	class error : public std::runtime_error {


		public:


			using std::runtime_error::runtime_error;


	};


	/**
	 *	Indicates that a transfer was aborted.
	 */
	class aborted : public error {


		public:


			aborted ();


	};


	/**
	 *	Represents an exception caused by the libcurl
	 *	multi interface.  Specifically an exception of
	 *	this type wraps a CURLMcode.
	 */
	class multi_error : public error {


		private:


			CURLMcode code_;


		public:


			multi_error () = delete;


			/**
			 *	Creates a multi_error which represents a certain
			 *	CURLMcode value.
			 *
			 *	\param [in] code
			 *		The CURLMcode value which the created object
			 *		shall represent.
			 */
			explicit multi_error (CURLMcode code);


			/**
			 *	Retrieves the CURLMcode which this object
			 *	represents.
			 *
			 *	\return
			 *		A CURLMcode.
			 */
			CURLMcode code () const noexcept;


	};


}
