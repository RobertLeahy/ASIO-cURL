/**
 *	\file
 */


#pragma once


namespace asiocurl {


	/**
	 *	An RAII class which calls curl_global_init when it is
	 *	constructed and curl_global_cleanup when its
	 *	lifetime ends.
	 */
	class init {


		public:


			init (const init &) = delete;
			init (init &&) = delete;
			init & operator = (const init &) = delete;
			init & operator = (init &&) = delete;


			/**
			 *	Calls curl_global_init with CURL_GLOBAL_ALL.
			 */
			init ();
			/**
			 *	Calls curl_global_init with custom flags.
			 *
			 *	\param [in] flags
			 *		The custom flags with which to call
			 *		curl_global_init.
			 */
			explicit init (long flags);
			/**
			 *	Calls curl_global_cleanup.
			 */
			~init () noexcept;


	};


}
