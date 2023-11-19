include(CheckIncludeFileCXX)

# check libexecinfo for non-glibc systems
include(CheckSymbolExists)
message("Checking if backtrace symbol present")
check_symbol_exists(backtrace execinfo.h backtrace_exists)
if(NOT backtrace_exists)
    message("backtrace is missing (non-glibc platform), trying to find libexecinfo")
    find_library(execinfo_LIB execinfo)
    if(NOT execinfo_LIB)
        message(FATAL_ERROR "libexecinfo not found")
    endif()
    set(OS_LIBRARIES ${execinfo_LIB})
else()
    set(SAYONARA_HAS_BACKTRACE 1)
endif()

# Check demangle
check_include_file_cxx("cxxabi.h" HAVE_CXX_ABI)
if(${HAVE_CXX_ABI})
    # Macros.h.in
    set(SAYONARA_HAS_CXX_ABI 1)
    message("Compile with demangle")
else()
    message("Demangle not found")
endif()