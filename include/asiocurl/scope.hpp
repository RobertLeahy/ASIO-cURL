#pragma once


#include "optional.hpp"
#include <exception>
#include <type_traits>
#include <utility>


namespace asiocurl {
	
	
	class scope_exit_policy {
		
		
		public:
		
		
			enum class when {
				
				success,
				fail,
				exit
				
			};
		
		
		private:
		
		
			when w_;
			
			
			bool check () const noexcept {
				
				if (w_==when::exit) return true;
				
				return std::uncaught_exception()==(w_==when::fail);
				
			}
			
			
		public:
		
		
			scope_exit_policy (const scope_exit_policy &) = default;
			scope_exit_policy (scope_exit_policy &&) = default;
			scope_exit_policy & operator = (const scope_exit_policy &) = delete;
			scope_exit_policy & operator = (scope_exit_policy &&) = delete;
			
			
			constexpr scope_exit_policy () noexcept : w_(when::exit) {	}
			constexpr scope_exit_policy (when w) noexcept : w_(w) {	}
			
			
			template <typename T>
			void operator () (T && f) const noexcept {
				
				if (!check()) return;
				if (!f) return;
				(*f)();
				
			}
		
		
	};
	
	
	template <typename EF>
	struct scope_exit {
		
		
		private:
			
			
			scope_exit_policy p_;
			optional<EF> f_;
			
			
		public:
		
		
			scope_exit () = delete;
			scope_exit (const scope_exit &) = delete;
			scope_exit & operator = (const scope_exit &) = delete;
			scope_exit & operator = (scope_exit &&) = delete;
			
			
			explicit scope_exit (const EF & f1, scope_exit_policy p=scope_exit_policy{}) noexcept : p_(p), f_(f1) {	}
			explicit scope_exit (EF && f2, scope_exit_policy p=scope_exit_policy{}) noexcept : p_(p), f_(std::move(f2)) {	}
			scope_exit (scope_exit && rhs) noexcept : p_(rhs.p_), f_(std::move(rhs.f_)) {
				
				rhs.f_=nullopt;
				
			}
			
			
			~scope_exit () noexcept {
				
				p_(f_);
				
			}
			
			
			void release () noexcept {
				
				f_=nullopt;
				
			}
			
			
			using type=scope_exit;
		
		
	};
	
	
	template <typename EF>
	struct scope_exit<EF &> {
		
		
		private:
		
		
			scope_exit_policy p_;
			EF * f_;
			
			
		public:
		
		
			scope_exit () = delete;
			scope_exit (const scope_exit &) = delete;
			scope_exit & operator = (const scope_exit &) = delete;
			scope_exit & operator = (scope_exit &&) = delete;
			
			
			explicit scope_exit (EF & f1, scope_exit_policy p=scope_exit_policy{}) noexcept : p_(p), f_(&f1) {	}
			scope_exit (EF &&) = delete;
			scope_exit (scope_exit && rhs) noexcept : p_(rhs.p_), f_(rhs.f_) {
				
				rhs.f_=nullptr;
				
			}
			
			
			~scope_exit () noexcept {
				
				p_(f_);
				
			}
			
			
			void release () noexcept {
				
				f_=nullptr;
				
			}
			
			
			using type=std::conditional_t<
				std::is_const<EF>::value,
				scope_exit<std::remove_const_t<EF>>,
				scope_exit
			>;
		
		
	};
	
	
	
	template <typename EF>
	struct scope_exit<EF &&> {
		
		
		public:
		
		
			scope_exit () = delete;
			
			
			using type=scope_exit<EF>;
		
		
	};
	
	
	template <typename EF>
	auto make_scope_exit (EF && exit_function, scope_exit_policy p) noexcept {
		
		return typename scope_exit<EF>::type(std::forward<EF>(exit_function),std::move(p));
		
	}
	template <typename EF>
	auto make_scope_exit (EF && exit_function) noexcept {
		
		return make_scope_exit(std::forward<EF>(exit_function),scope_exit_policy::when::exit);
		
	}
	template <typename EF>
	auto make_scope_fail (EF && exit_function) noexcept {
		
		return make_scope_exit(std::forward<EF>(exit_function),scope_exit_policy::when::fail);
		
	}
	template <typename EF>
	auto make_scope_success (EF && exit_function) noexcept {
		
		return make_scope_exit(std::forward<EF>(exit_function),scope_exit_policy::when::success);
		
	}
	
	
}
