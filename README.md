# ASIO cURL [![Build Status](https://travis-ci.org/RobertLeahy/ASIO-cURL.svg?branch=master)](https://travis-ci.org/RobertLeahy/ASIO-cURL)

ASIO cURL is a C++14 library which provides asynchronous access to the facilities of [libcurl](https://curl.haxx.se/libcurl/) using [Boost.ASIO](http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio.html).

## Why ASIO cURL?

Because I couldn't find anything else [other than this](https://curl.haxx.se/libcurl/c/asiohiper.html) (which is hardly adequate).

## What is ASIO cURL?

ASIO cURL is not intended to be a C++ wrapper for libcurl, rather it is intended to provide features which allow you to gain all the advantages of Boost.ASIO and the [libcurl multi interface](https://curl.haxx.se/libcurl/c/libcurl-multi.html) while never having to make a single `curl_multi_*` call.

Code that uses ASIO cURL will still use the C-style `curl_easy_*` calls.  This is to avoid limiting client code by failing to abstract a certain niche feature or option of libcurl (or by failing to stay up-to-date as libcurl adds new features, options, et cetera).

That said three helper classes for the [libcurl easy interface](https://curl.haxx.se/libcurl/c/libcurl-easy.html) are provided:

- `asiocurl::easy` which wraps a `CURL *` in a C++ class (with, of course, RAII)
- `asiocurl::easy_with_error_buffer` which provides all the convenience of `asiocurl::easy` plus automatic use of [`CURLOPT_ERRORBUFFER`](https://curl.haxx.se/libcurl/c/CURLOPT_ERRORBUFFER.html)
- `asiocurl::easy_error` an exception type which represents a `CURLcode`

However their use is completely optional to use ASIO cURL.  All client-facing edges of the core ASIO cURL class (`asiocurl::io_service`) deal with raw `CURL *` objects.

## Using ASIO cURL

To use ASIO cURL you must:

1. Create a `boost::asio::io_service`
2. Create an `asiocurl::io_service`
3. Create and setup a libcurl easy handle
4. Call `asiocurl::io_service::add` and pass the easy handle from 3
5. Ensure `boost::asio::io_service::run`, `boost::asio::io_service::run_one`, `boost::asio::io_service::poll`, and/or `boost::asio::io_service::poll_one` are being called
6. Wait on the future from 4 to get a `CURLMsg` structure which represents the final result of your transfer

## Example

```
//	Setup Boost.ASIO
boost::asio::io_service ios;
std::experimental::optional<boost::asio::io_service::work> work(std::experimental::in_place,ios);
std::thread t([&] () { ios.run(); });

//	Setup libcurl (i.e. call curl_global_init)
asiocurl::init init;

//	Setup ASIO cURL
asiocurl::io_service curl;

//	Setup easy handle for HTTP transfer
asiocurl::easy_with_error_buffer easy;
const char * url="http://google.com";
auto result=curl_easy_setopt(easy,CURLOPT_URL,url);
if (result!=CURLE_OK) easy.raise(result);

//	Run
auto msg=curl.add(easy).get();
```

## Dependencies

- Boost.ASIO
- libcurl

## Supported Compilers

The following compilers have been tested:

### Linux

- Clang 3.8
- GCC 5.3.0
- GCC 6.1.1

### Windows

- GCC 5.3.0

## Build

### Linux

```
cmake .
make
```

### Windows

```
cmake -G "MinGW Makefiles" .
make
```

If you prefer to use `boost::future` rather than `std::future` call CMake with `-DUSE_BOOST_FUTURE=1`.

If you would like to build and run the tests call CMake with `-DBUILD_TESTS=1`.  The tests add [Catch](https://github.com/philsquared/Catch) as a dependency.  The tests will be built and run automatically if you build in debug mode.

## Documentation

To full build documentation simply run [`doxygen`](http://www.stack.nl/~dimitri/doxygen/).