add_library(co_liburing INTERFACE)
cmake_policy(SET CMP0076 NEW)
target_sources(co_liburing INTERFACE liburing.cpp)
target_include_directories(co_liburing INTERFACE include)
