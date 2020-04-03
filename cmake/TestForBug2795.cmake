# This file is a CMake configuration source compile check
#
# G++ 10.0.1 had a regression bug, which causes boost::serialization to fail
# to compile because the underlying type of a strong typedef template did not
# expose its ++/-- post operators.
#
# For more details see:
#   https://github.com/freeorion/freeorion/issues/2795
#   https://github.com/boostorg/serialization/issues/192
#   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=94190


include(CheckCXXSourceCompiles)

# Enforce compiler to be invoked with C locale to ensure non-localized
# error messages.
set(_locale_vars LC_ALL LC_MESSAGES LANG)
foreach(v IN LISTS _locale_vars)
    set(_locale_vars_saved_${v} "$ENV{${v}}")
    set(ENV{${v}} C)
endforeach()

check_cxx_source_compiles("
struct A
{
    int _i;
    operator int&() { return _i; }
};

template <typename T> void f()
{
    A a;
    a++; // triggers bug
}

int main() { f<int>(); }
" NOT_GNU_CXX_BUG_94190 FAIL_REGEX "no post-decrement operator for type")

if(NOT NOT_GNU_CXX_BUG_94190)
    message(FATAL_ERROR
        "The found version of the g++ compiler suffers from bug"
        " https://gcc.gnu.org/bugzilla/show_bug.cgi?id=94190"
        " which triggers a known bug in combination with boost::serialization."
        " Please update the compiler to a more recent version of g++ or"
        " use an alternative compiler."
        " After update rebuild the project from a clean build tree."
    )
endif()

# Restore locale settings
foreach(v IN LISTS _locale_vars)
    set(ENV{${v}} ${_locale_vars_saved_${v}})
endforeach()
