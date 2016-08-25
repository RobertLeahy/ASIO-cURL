#include <asiocurl/scope.hpp>


#include <asiocurl/optional.hpp>
#include <utility>
#include <catch.hpp>


namespace {
	
	
	class functor {
		
		
		private:
		
		
			bool * ptr_;
			bool i_;
			
			
		public:
		
		
			functor () = delete;
			functor (bool & b) noexcept : ptr_(&b), i_(false) {	}
			
			
			explicit operator bool () const noexcept {
				
				//	Determines whether this object has been
				//	invoked directly
				return i_;
				
			}
			
			
			void operator () () noexcept {
				
				i_=true;
				if (ptr_) *ptr_=true;
				
			}
		
		
	};
	
	
}


template <typename T>
static void fail (asiocurl::scope_exit<T> e) noexcept {
	
	try {
		
		asiocurl::scope_exit<T> i(std::move(e));
		
		throw 0;
		
	} catch (...) {	}
	
}


SCENARIO("scope_exit objects invoke their stored functor when they go out of scope unless they are moved or released","[asiocurl][scope_exit][scope]") {
	
	using asiocurl::in_place;
	using asiocurl::nullopt;
	using asiocurl::optional;
	using asiocurl::scope_exit;
	
	bool i=false;
	functor f(i);
	
	GIVEN("A scope_exit object which is not templated on a reference type") {
		
		optional<scope_exit<functor>> e(in_place,f);
		
		WHEN("It goes out of scope") {
			
			e=nullopt;
			
			THEN("The stored functor is invoked") {
				
				CHECK(i);
				CHECK(!f);
				
			}
			
		}
		
		WHEN("It goes out of scope due to an exception") {
			
			fail(std::move(*e));
			
			THEN("The stored functor is invoked") {
				
				CHECK(i);
				CHECK(!f);
				
			}
			
		}
		
		WHEN("It is released") {
			
			e->release();
			
			AND_WHEN("It goes out of scope") {
				
				e=nullopt;
				
				THEN("The stored functor is not invoked") {
					
					CHECK(!i);
					
				}
				
			}
			
		}
		
		WHEN("It is moved") {
			
			optional<scope_exit<functor>> mt(in_place,std::move(*e));
			
			AND_WHEN("The moved from scope_exit object goes out of scope") {
				
				e=nullopt;
				
				THEN("The stored functor is not invoked") {
					
					CHECK(!i);
					
				}
				
				AND_WHEN("The moved to scope_exit object goes out of scope") {
					
					mt=nullopt;
					
					THEN("The stored functor is invoked") {
						
						CHECK(i);
						CHECK(!f);
						
					}
					
				}
				
			}
			
		}
		
	}
	
	GIVEN("A scope_exit object which is templated on a reference type") {
		
		optional<scope_exit<functor &>> e(in_place,f);
		
		WHEN("It goes out of scope") {
			
			e=nullopt;
			
			THEN("The functor (not a copy thereof) is invoked") {
				
				CHECK(i);
				CHECK(f);
				
			}
			
		}
		
		WHEN("It goes out of scope due to an exception") {
			
			fail(std::move(*e));
			
			THEN("The functor is invoked") {
				
				CHECK(i);
				CHECK(f);
				
			}
			
		}
		
		WHEN("It is released") {
			
			e->release();
			
			AND_WHEN("It goes out of scope") {
				
				e=nullopt;
				
				THEN("The functor is not invoked") {
					
					CHECK(!i);
					
				}
				
			}
			
		}
		
		WHEN("It is moved") {
			
			optional<scope_exit<functor &>> mt(in_place,std::move(*e));
			
			AND_WHEN("The moved from scope_exit object goes out of scope") {
				
				e=nullopt;
				
				THEN("The functor is not invoked") {
					
					CHECK(!i);
					
				}
				
				AND_WHEN("The moved to scope_exit object goes out of scope") {
					
					mt=nullopt;
					
					THEN("The functor (not a copy thereof) is invoked") {
						
						CHECK(i);
						CHECK(f);
						
					}
					
				}
				
			}
			
		}
		
	}
	
}


SCENARIO("scope_exit objects obtained from the make_scope_exit factory invoke their stored functor when they go out of scope","[asiocurl][scope_exit][scope]") {
	
	using asiocurl::make_scope_exit;
	using asiocurl::make_scope_fail;
	using asiocurl::make_scope_success;
	using asiocurl::nullopt;
	using asiocurl::scope_exit;
	using asiocurl::optional;
	
	bool i=false;
	functor f(i);
	
	GIVEN("A scope_exit object obtained from the make_scope_exit factory called with an rvalue") {
		
		optional<scope_exit<functor>> e(make_scope_exit(std::move(f)));
		
		WHEN("It goes out of scope") {
			
			e=nullopt;
			
			THEN("The stored functor is invoked") {
				
				CHECK(i);
				CHECK(!f);
				
			}
			
		}
		
		WHEN("It goes out of scope due to an exception") {
			
			fail(std::move(*e));
			
			THEN("The stored functor is invoked") {
				
				CHECK(i);
				CHECK(!f);
				
			}
			
		}
		
	}
	
	GIVEN("A scope_exit object obtained from the make_scope_exit factory called with a const lvalue reference") {
		
		const auto & r=f;
		optional<scope_exit<functor>> e(make_scope_exit(r));
		
		WHEN("It goes out of scope") {
			
			e=nullopt;
			
			THEN("The stored functor is invoked") {
				
				CHECK(i);
				CHECK(!f);
				
			}
			
		}
		
		WHEN("It goes out of scope due to an exception") {
			
			fail(std::move(*e));
			
			THEN("The stored functor is invoked") {
				
				CHECK(i);
				CHECK(!f);
				
			}
			
		}
		
	}
	
	GIVEN("A scope_exit object obtained from the make_scope_exit factory called with a mutable lvalue reference to a functor") {
		
		optional<scope_exit<functor &>> e(make_scope_exit(f));
		
		WHEN("It goes out of scope") {
			
			e=nullopt;
			
			THEN("The functor (not a copy thereof) is invoked") {
				
				CHECK(i);
				CHECK(f);
				
			}
			
		}
		
		WHEN("It goes out of scope due to an exception") {
			
			fail(std::move(*e));
			
			THEN("The functor (not a copy thereof) is invoked") {
				
				CHECK(i);
				CHECK(f);
				
			}
			
		}
		
	}
	
	GIVEN("A scope_exit object obtained from the make_scope_fail factory called with an rvalue") {
		
		optional<scope_exit<functor>> e(make_scope_fail(std::move(f)));
		
		WHEN("It goes out of scope") {
			
			e=nullopt;
			
			THEN("The stored functor is not invoked") {
				
				CHECK(!i);
				
			}
			
		}
		
		WHEN("It goes out of scope due to an exception") {
			
			fail(std::move(*e));
			
			THEN("The stored functor is invoked") {
				
				CHECK(i);
				CHECK(!f);
				
			}
			
		}
		
	}
	
	GIVEN("A scope_exit object obtained from the make_scope_fail factory called with a const lvalue reference") {
		
		const auto & r=f;
		optional<scope_exit<functor>> e(make_scope_fail(r));
		
		WHEN("It goes out of scope") {
			
			e=nullopt;
			
			THEN("The stored functor is not invoked") {
				
				CHECK(!i);
				
			}
			
		}
		
		WHEN("It goes out of scope due to an exception") {
			
			fail(std::move(*e));
			
			THEN("The stored functor is invoked") {
				
				CHECK(i);
				CHECK(!f);
				
			}
			
		}
		
	}
	
	GIVEN("A scope_exit object obtained from the make_scope_fail factory called with a mutable lvalue reference to a functor") {
		
		optional<scope_exit<functor &>> e(make_scope_fail(f));
		
		WHEN("It goes out of scope") {
			
			e=nullopt;
			
			THEN("The functor is not invoked") {
				
				CHECK(!i);
				
			}
			
		}
		
		WHEN("It goes out of scope due to an exception") {
			
			fail(std::move(*e));
			
			THEN("The functor (not a copy thereof) is invoked") {
				
				CHECK(i);
				CHECK(f);
				
			}
			
		}
		
	}
	
	GIVEN("A scope_exit object obtained from the make_scope_success factory called with an rvalue") {
		
		optional<scope_exit<functor>> e(make_scope_success(std::move(f)));
		
		WHEN("It goes out of scope") {
			
			e=nullopt;
			
			THEN("The stored functor is invoked") {
				
				CHECK(i);
				CHECK(!f);
				
			}
			
		}
		
		WHEN("It goes out of scope due to an exception") {
			
			fail(std::move(*e));
			
			THEN("The stored functor is not invoked") {
				
				CHECK(!i);
				
			}
			
		}
		
	}
	
	GIVEN("A scope_exit object obtained from the make_scope_success factory called with a const lvalue reference") {
		
		const auto & r=f;
		optional<scope_exit<functor>> e(make_scope_success(r));
		
		WHEN("It goes out of scope") {
			
			e=nullopt;
			
			THEN("The stored functor is invoked") {
				
				CHECK(i);
				CHECK(!f);
				
			}
			
		}
		
		WHEN("It goes out of scope due to an exception") {
			
			fail(std::move(*e));
			
			THEN("The stored functor is not invoked") {
				
				CHECK(!i);
				
			}
			
		}
		
	}
	
	GIVEN("A scope_exit object obtained from the make_scope_success factory called with a mutable lvalue reference to a functor") {
		
		optional<scope_exit<functor &>> e(make_scope_success(f));
		
		WHEN("It goes out of scope") {
			
			e=nullopt;
			
			THEN("The functor (not a copy thereof) is invoked") {
				
				CHECK(i);
				CHECK(f);
				
			}
			
		}
		
		WHEN("It goes out of scope due to an exception") {
			
			fail(std::move(*e));
			
			THEN("The functor is not invoked") {
				
				CHECK(!i);
				
			}
			
		}
		
	}
	
}
