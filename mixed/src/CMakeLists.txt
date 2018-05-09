cmake_minimum_required (VERSION 3.6)
project(app)

include(GenerateExportHeader)

find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy"
  DOC "Path to clang-tidy executable"
  )
if(NOT CLANG_TIDY_EXE)
  message(SEND_ERROR "clang-tidy not found.")
else()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

link_directories(${CMAKE_INSTALL_PREFIX}/lib/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR -D_LIBCPP_ENABLE_CXX17_REMOVED_RANDOM_SHUFFLE -fcoroutines-ts -Wno-missing-prototypes")
set(LINK_FLAGS "${LINK_FLAGS} -rdynamic -g -fno-omit-frame-pointer -fno-limit-debug-info")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-omit-frame-pointer -fno-limit-debug-info -ftemplate-backtrace-limit=0")

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  set(LINK_FLAGS "${LINK_FLAGS} -O0 -fprofile-instr-generate -fcoverage-mapping")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -DSPDLOG_DEBUG_ON")
else()
  set(LINK_FLAGS "${LINK_FLAGS} -O3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

if (${OPT_ENABLE_PROFILING} MATCHES 1)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
endif()

#module, static, shared, exe
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINK_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINK_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINK_FLAGS}")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

#Test
add_executable(app_test
    test.cpp
    presentation_examples_test.cpp
    task.cpp
    sync/main.cpp
    threaded/main.cpp
    callback/main.cpp
    task/main.cpp
    boost_coroutine/main.cpp
    coroutine_ts/main.cpp
)
target_link_libraries(app_test PRIVATE
  ${CMAKE_INSTALL_PREFIX}/lib/libboost_system.a
  ${CMAKE_INSTALL_PREFIX}/lib/libboost_filesystem.a
  ${CMAKE_INSTALL_PREFIX}/lib/libboost_thread.a
  ${CMAKE_INSTALL_PREFIX}/lib/libboost_chrono.a
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
  -I${CMAKE_INSTALL_PREFIX}/include/
  -I/home/thingdust/prebuilt/src/json/src/
  -I/home/thingdust/prebuilt/src/spdlog/include/
  -I/home/thingdust/prebuilt/src/GSL/include/
  -I/home/thingdust/prebuilt/src/RxCpp/Rx/v2/src/
  -I/home/thingdust/boost/
  --system-header-prefix=boost/
  --system-header-prefix=gtest/
  --system-header-prefix=gmock/
  --system-header-prefix=google/
  --system-header-prefix=api/
  --system-header-prefix=gsl/
  --system-header-prefix=rxcpp/
  --system-header-prefix=spdlog/
  --system-header-prefix=unicode/

)

if(${USE_CLANG_TIDY} MATCHES YES)
  set_target_properties(app_test PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
endif()
