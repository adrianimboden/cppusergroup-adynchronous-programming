cmake_minimum_required (VERSION 3.9)
project(app)

link_directories(${CMAKE_INSTALL_PREFIX}/lib/)

add_executable(app_test
    test.cpp
    task.cpp
    01_sync/main.cpp
    02_threaded/main.cpp
    03_callback/main.cpp
    04_task/main.cpp
    05_boost_coroutine/main.cpp
    06_coroutine_ts/main.cpp
)

target_link_libraries(app_test PRIVATE
  ${CMAKE_INSTALL_PREFIX}/lib/libboost_system.a
  ${CMAKE_INSTALL_PREFIX}/lib/libboost_thread.a
  ${CMAKE_INSTALL_PREFIX}/lib/libboost_context.a

  gtest
  gmock
  gtest_main
  pthread
)

target_include_directories(app_test PRIVATE
  src/
)

target_compile_options(app_test PRIVATE
  -Weverything
  -Werror
  -Wno-user-defined-warnings
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-padded
  -Wno-weak-vtables
  -Wno-global-constructors
  -Wno-exit-time-destructors
  -Wno-undefined-func-template
  -Wno-newline-eof
  -Wno-zero-as-null-pointer-constant
  -Wno-missing-prototypes
  -I${CMAKE_INSTALL_PREFIX}/include/
  -I/prebuilt/src/GSL/include/
  --system-header-prefix=boost/
  --system-header-prefix=gtest/
  --system-header-prefix=gmock/
  --system-header-prefix=gsl/
  -fcoroutines-ts
)
