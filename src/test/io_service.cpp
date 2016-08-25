#include <asiocurl/io_service.hpp>


#include <asiocurl/easy.hpp>
#include <asiocurl/exception.hpp>
#include <asiocurl/optional.hpp>
#include <boost/asio.hpp>
#include <curl/curl.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <catch.hpp>


namespace {


	class fixture {


		protected:


			boost::asio::io_service ios;


		private:


			asiocurl::optional<asiocurl::io_service> opt_;


		protected:


			asiocurl::io_service & curl;


		public:


			fixture () : opt_(asiocurl::in_place,ios), curl(*opt_) {	}


			void shutdown () noexcept {

				opt_=asiocurl::nullopt;

			}


	};


	class runner {


		private:


			asiocurl::optional<boost::asio::io_service::work> work_;
			std::thread t_;


		public:


			runner () = delete;
			runner (const runner &) = delete;
			runner (runner &&) = delete;
			runner & operator = (const runner &) = delete;
			runner & operator = (runner &&) = delete;


			explicit runner (boost::asio::io_service & ios) : work_(asiocurl::in_place,ios) {

				t_=std::thread([&ios] () noexcept {

					ios.run();

				});

			}


			~runner () noexcept {

				work_=asiocurl::nullopt;
				t_.join();

			}


	};


}


static std::string url (unsigned code) {

	std::ostringstream ss;
	ss << "http://httpbin.org/status/" << code;

	return ss.str();

}


static std::string timeout (unsigned seconds) {

	std::ostringstream ss;
	ss << "http://httpbin.org/delay/" << seconds;

	return ss.str();

}


template <typename T>
static void set (asiocurl::easy::native_handle_type easy, CURLoption option, T param) {

	auto result=curl_easy_setopt(easy,option,param);
	if (result!=CURLE_OK) throw asiocurl::easy_error(result);

}


SCENARIO_METHOD(fixture,"asiocurl::io_service::add rejects duplicate easy handles","[asiocurl][io_service]") {

	GIVEN("A curl easy handle") {

		asiocurl::easy easy;

		WHEN("It is added to an asiocurl::io_service") {

			curl.add(easy);

			THEN("Attempting to add it again throws an exception") {

				CHECK_THROWS_AS(curl.add(easy),std::logic_error);

				AND_THEN("The handle is still managed by the asiocurl::io_service") {

					CHECK(curl.remove(easy));

				}

			}

		}

	}

}


SCENARIO_METHOD(fixture,"asiocurl::io_service::remove may be used to abort transfers","[asiocurl][io_service]") {

	GIVEN("A curl easy handle") { 

		asiocurl::easy easy;

		THEN("Passing it to asiocurl::io_service::remove returns false") {

			CHECK_FALSE(curl.remove(easy));

		}

		WHEN("It is added to an asiocurl::io_service") {

			auto f=curl.add(easy);

			THEN("Passing it to asiocurl::io_service::remove returns true") {

				REQUIRE(curl.remove(easy));

				AND_THEN("The future which represents the result of that transfer throws an exception when an attempt is made to retrieve its value") {

					CHECK_THROWS_AS(f.get(),asiocurl::aborted);

				}

			}

		}

	}

}


SCENARIO_METHOD(fixture,"When the lifetime of an asiocurl::io_service object ends all transfers are aborted","[asiocurl][io_service]") {

	GIVEN("A curl easy handle") {

		asiocurl::easy easy;

		WHEN("It is added to an asiocurl::io_service") {

			auto f=curl.add(easy);

			AND_WHEN("The lifetime of that asiocurl::io_service object ends") {

				shutdown();

				THEN("The transfer is aborted") {

					CHECK_THROWS_AS(f.get(),asiocurl::aborted);

				}
				
			}

		}

	}

}


SCENARIO_METHOD(fixture,"Transfers managed by an asiocurl::io_service object complete","[asiocurl][io_service]") {

	GIVEN("A curl easy handle which represents a transfer which should complete") {

		asiocurl::easy easy;
		auto u=url(200);
		set(easy,CURLOPT_URL,u.c_str());
		long v=1;
		set(easy,CURLOPT_VERBOSE,v);

		WHEN("It is added to an asiocurl::io_service and boost::asio::io_service::run is invoked") {

			auto f=curl.add(easy);
			runner r(ios);

			THEN("The transfer completes") {

				auto s=f.get();

				AND_THEN("It completed successfully") {

					CHECK(s.data.result==CURLE_OK);

				}

				AND_THEN("The returned CURLMsg is for the appropriate easy handle") {

					CHECK(s.easy_handle==easy.native_handle());
					
				}

				AND_THEN("The easy handle is no longer managed by the asiocurl::io_service") {

					CHECK_FALSE(curl.remove(easy));

				}

				AND_THEN("The returned CURLMsg has CURLMSG_DONE") {

					CHECK(s.msg==CURLMSG_DONE);

				}

				AND_THEN("The result of the transfer is correct") {

					long code;
					auto result=curl_easy_getinfo(easy,CURLINFO_RESPONSE_CODE,&code);
					if (result!=CURLE_OK) throw asiocurl::easy_error(result);
					CHECK(code==200);

				}

			}

		}

	}

	GIVEN("A curl easy handle which represents a transfer which should timeout") {

		asiocurl::easy easy;
		auto u=timeout(2);
		set(easy,CURLOPT_URL,u.c_str());
		long v=1;
		set(easy,CURLOPT_VERBOSE,v);
		long timeout_ms=750;
		set(easy,CURLOPT_TIMEOUT_MS,timeout_ms);

		WHEN("It is added to an asiocurl::io_service and boost::asio::io_service::run is invoked") {

			auto f=curl.add(easy);
			runner r(ios);

			THEN("The transfer completes") {

				auto s=f.get();

				AND_THEN("It timed out") {

					CHECK(s.data.result==CURLE_OPERATION_TIMEDOUT);

				}

				AND_THEN("The returned CURLMsg is for the appropriate easy handle") {

					CHECK(s.easy_handle==easy.native_handle());
					
				}

				AND_THEN("The easy handle is no longer managed by the asiocurl::io_service") {

					CHECK_FALSE(curl.remove(easy));

				}

				AND_THEN("The returned CURLMsg has CURLMSG_DONE") {

					CHECK(s.msg==CURLMSG_DONE);

				}

			}

		}

	}

}
