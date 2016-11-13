cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(concurrent_unbounded_queue VERSION 1.0 LANGUAGES CXX C)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_executable(concurrent_unbounded_queue
  main.cc
  utils/lock_based_queue.h
  utils/lock_free_queue.h
  utils/queue_node.h
  utils/tester.h
  utils/log_util.h)

target_include_directories(concurrent_unbounded_queue PRIVATE utils)

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(concurrent_unbounded_queue PRIVATE -std=c++0x)
  target_link_libraries(concurrent_unbounded_queue PRIVATE Threads::Threads)
endif()
