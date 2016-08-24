#include <asiocurl/easy.hpp>


#include <asiocurl/exception.hpp>
#include <asiocurl/io_service.hpp>
#include <asiocurl/optional.hpp>
#include <boost/asio.hpp>
#include <curl/curl.h>
#include <thread>
#include <catch.hpp>


namespace {


	class fixture {


		protected:


			boost::asio::io_service ios;
			asiocurl::io_service curl;


		private:

			
			asiocurl::optional<boost::asio::io_service::work> work_;
			std::thread t_;


		public:


			fixture () : curl(ios), work_(asiocurl::in_place,ios) {

				t_=std::thread([&] () noexcept {	ios.run();	});

			}


			~fixture () noexcept {

				work_=asiocurl::nullopt;
				t_.join();

			}


	};


}


SCENARIO_METHOD(fixture,"asiocurl::easy_with_error_buffer captures detailed error messages from libcurl","[asiocurl][easy][easy_with_error_buffer]") {

	GIVEN("An asiocurl::easy_with_error_buffer object which represents a transfer which will fail") {

		asiocurl::easy_with_error_buffer easy;
		const char * str="http://foo.corge";	//	NXDOMAIN
		auto result=curl_easy_setopt(easy,CURLOPT_URL,str);
		if (result!=CURLE_OK) throw asiocurl::easy_error(result);
		long v=1;
		if ((result=curl_easy_setopt(easy,CURLOPT_VERBOSE,v))!=CURLE_OK) throw asiocurl::easy_error(result);

		WHEN("The transfer is started") {

			auto f=curl.add(easy);

			THEN("It completes") {

				auto s=f.get();

				AND_THEN("The transfer failed") {

					CHECK(s.data.result==CURLE_COULDNT_RESOLVE_HOST);

					AND_THEN("The asio::easy_with_error_buffer contains a detailed error message") {

						CHECK(!easy.empty());

					}

				}

			}

		}

	}

}
