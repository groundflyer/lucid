include(CheckCXXSourceCompiles)
include(CheckCXXCompilerFlag)

set(CMAKE_REQUIRED_FLAGS -std=c++17)

# Check for Typename in template
check_cxx_source_compiles("template <template <typename> typename T> struct foo {}; int main(){return 0;}" Typename_In_Template)

if(NOT Typename_In_Template)
  message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION} doesn't support typename in a template (N4051)")
endif()

# Check for Structured bindings
check_cxx_source_compiles("struct foo{int a; char b;}; int main(){auto [a, b] = foo{}; return 0;}" Structured_Bindings)

if(NOT Structured_Bindings)
  message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION} doesn't support structured bindings (P0217R3)")
endif()

check_cxx_source_compiles("template <typename T> struct St {T val;};
template <int I, typename T> constexpr auto func(T s) { return func<I>(St<T>{T{I}}); }
template <int I, typename T, template <typename> typename S> constexpr auto func(S<T> s) { return s.val + T{I} - I; }
static_assert(func<1>(1) == 1);
int main(){ constexpr auto a = func<1>(1); return a == 1; }" Template_Template_Overload)

if(NOT Template_Template_Overload)
  message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION} doesn't support template template overloading")
endif()

check_cxx_source_compiles("#include <functional>
int main() { return std::invoke([](){ return 0; }); }" Std_Invoke)

if(NOT Std_Invoke)
  message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}'s STL doesn't have std::invoke")
endif()

check_cxx_source_compiles("int main(){auto f1 = [](int a){return a + 1;};decltype(f1) f2;f1 = f2;return 0;}" Default_Constructible_Assignable_Lambdas)
if (NOT Default_Constructible_Assignable_Lambdas)
  message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION} doesn't support default constructible and assignable stateless lambdas (P0624R2)")
endif()

check_cxx_source_compiles("#include <utility>
template <typename T> constexpr auto maker() noexcept{
return []<typename... Args>(Args && ... args) constexpr {return T(std::forward<Args>(args)...); };}
int main(){auto a = maker<int>(); return a(0);}" Template_Parameters_For_Lambdas)
if (NOT Template_Parameters_For_Lambdas)
  message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION} doesn't support template parameter syntax for generic lambdas (P0428R2)")
endif()
