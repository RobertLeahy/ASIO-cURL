cmake_minimum_required(VERSION 2.6)
project(ModuleLoader LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(CheckIncludeFileCXX)

function(add_linker_options)
	foreach(arg ${ARGV})
		set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} ${arg}")
		set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES}" PARENT_SCOPE)
	endforeach(arg)
endfunction()

#	Where our project's headers live
include_directories(include)

#	General command line arguments to the compiler that should be present
#	on all platforms
add_compile_options(-std=c++1z)
if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_options(-O0 -g -fno-inline -fno-omit-frame-pointer -Wall -Wpedantic -Wextra -Werror)
	#	Address Sanitizer (non-Windows only)
	if(NOT WIN32)
		#	Address Sanitizer needs to be able to be turned off due
		#	to the fact the AMD OpenCL CPU SDK doesn't play nice with
		#	it
		if(NOT DEFINED USE_ADDRESS_SANITIZER OR USE_ADDRESS_SANITIZER)
			add_compile_options(-fsanitize=address)
			add_linker_options(-fsanitize=address)
		endif()
	endif()
	#	Enable all warnings except a choice few on Clang
	if (DEFINED CMAKE_CXX_COMPILER_ID AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		add_compile_options(-Weverything)
		#	We don't care about C++98 compatibility: This is a C++14
		#	library
		add_compile_options(-Wno-c++98-compat)
		#	These warnings are just noise
		add_compile_options(-Wno-weak-vtables -Wno-padded)
		#	curl_easy_setopt is apparently implemented in terms of
		#	a recursive macro which sets this warning off
		add_compile_options(-Wno-disabled-macro-expansion)
	endif()
else()
	add_compile_options(-O2)
endif()

if(NOT DEFINED USE_BOOST_FUTURE)
	set(USE_BOOST_FUTURE 1)
endif()

configure_file(src/configure.hpp.in include/asiocurl/configure.hpp)

add_library(asiocurl SHARED
	src/easy.cpp
	src/exception.cpp
	src/init.cpp
	src/io_service.cpp
)
target_link_libraries(asiocurl curl boost_system)
if (WIN32)
	target_link_libraries(asiocurl curldll ws2_32)
endif()
if(USE_BOOST_FUTURE)
	target_link_libraries(asiocurl boost_thread)
endif()

if((DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug") OR (DEFINED BUILD_TESTS AND BUILD_TESTS))
	add_executable(tests
		src/test/easy.cpp
		src/test/io_service.cpp
		src/test/main.cpp
		src/test/scope.cpp
	)
	target_link_libraries(tests asiocurl)
	if(NOT WIN32)
		target_link_libraries(tests pthread)
	endif()
	#	Catch triggers -Wexit-time-destructors like crazy
	#	so we turn it off only for the tests
	if (DEFINED CMAKE_CXX_COMPILER_ID AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(tests PRIVATE -Wno-exit-time-destructors)
	endif()
	add_custom_target(tests_run ALL
		COMMAND tests
		DEPENDS tests
		WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
		COMMENT "Run test suite"
	)
endif()
