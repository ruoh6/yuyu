cmake_minimum_required(VERSION 3.0)
project(yuyu)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

#set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")
# set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

include(cmake/utils.cmake)
include_directories(.)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdpart)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdpart/openssl-1.0.1/build/install/include/)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdpart/openssl-1.0.1/build/install/lib/)

# set module path:./cmake
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# other way set module path 
# list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# find_package(Boost REQUIRED)
# if(Boost_FOUND)
#     include_directories(${Boost_INCLUDE_DIRS})
# endif()
# 
# find_package(MYSQL REQUIRED)
# if(MYSQL_FOUND)
#     include_directories(${MYSQL_INCLUDE_DIRS})
# endif()
# 
# find_package(Protobuf REQUIRED)
# if({Protobuf_FOUND)
#     include_directories(${Protobuf_INCLUDE_DIRS})
# endif()
# 
# find_package(OpenSSL REQUIRED)
# if(OpenSSL_FOUND)
#     include_directories(${OpenSSL_INCLUDE_DIRS})
# endif()
# 
# 
# find_package(ZLIB REQUIRED)
# if(ZLIB_FOUND)
#link_directories(/usr/local/lib)
#     include_directories(${ZLIB_INCLUDE_DIRS})
# endif()
#find_package(GTest REQUIRED)

find_package(yaml-cpp REQUIRED)
if(YAMLCPP_FOUND)
    include_directories(${YAMLCPP_INCLUDE_DIRS})
endif()
set(LIB_SRC
    yuyu/address.cc
    yuyu/bytearray.cc
    yuyu/config.cc
    yuyu/hook.cc
    yuyu/http/http.cc
    yuyu/http/http_parser.cc
    yuyu/http/http_session.cc
    yuyu/http/http_server.cc
    yuyu/http/http_connection.cc
    yuyu/http/servlet.cc
    yuyu/log.cc
    yuyu/util.cc
    yuyu/tcp_server.cc
    yuyu/thread.cc
    yuyu/timer.cc
    yuyu/iomanager.cc
    yuyu/fiber.cc
    yuyu/fd_manager.cc
    yuyu/socket.cc
    yuyu/stream.cc
    yuyu/streams/socket_stream.cc
    yuyu/scheduler.cc
    )

ragelmaker(yuyu/http/http11_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/yuyu/http)
ragelmaker(yuyu/http/httpclient_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/yuyu/http)
ragelmaker(yuyu/uri.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/yuyu)

 set(LIBS
     yuyu
     dl
     pthread
     yaml-cpp
     ssl
     crypto
#     jsoncpp
#     event
#     hiredis_vip
#     mysqlclient_r
#     zookeeper_mt
#     sqlite3
#     tinyxml2
#     jemalloc
     )

add_library(yuyu SHARED ${LIB_SRC})

# Example
yuyu_add_executable(test_log "tests/test_log.cc" yuyu "${LIBS}")
yuyu_add_executable(test_config "tests/test_config.cc" yuyu "${LIBS}")
yuyu_add_executable(test_thread "tests/test_thread.cc" yuyu "${LIBS}")
yuyu_add_executable(test_util "tests/test_util.cc" yuyu "${LIBS}")
yuyu_add_executable(test_fiber "tests/test_fiber.cc" yuyu "${LIBS}")
yuyu_add_executable(test_scheduler "tests/test_scheduler.cc" yuyu "${LIBS}")
yuyu_add_executable(test_iomanager "tests/test_iomanager.cc" yuyu "${LIBS}")
yuyu_add_executable(test_hook "tests/test_hook.cc" yuyu "${LIBS}")
yuyu_add_executable(test_address "tests/test_address.cc" yuyu "${LIBS}")
yuyu_add_executable(test_socket "tests/test_socket.cc" yuyu "${LIBS}")
yuyu_add_executable(test_bytearray "tests/test_bytearray.cc" yuyu "${LIBS}")
yuyu_add_executable(test_tcp_server "tests/test_tcp_server.cc" yuyu "${LIBS}")
yuyu_add_executable(test_http "tests/test_http.cc" yuyu "${LIBS}")
yuyu_add_executable(example_http_parser "example/example_http_parser.cc" yuyu "${LIBS}")
yuyu_add_executable(test_http_server "tests/test_http_server.cc" yuyu "${LIBS}")
yuyu_add_executable(test_http_connection "tests/test_http_connection.cc" yuyu "${LIBS}")


# Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(run_tests
    tests/test_http_parser.cc
)

target_link_libraries(run_tests
     yuyu
     dl
     pthread
     yaml-cpp
     ssl
     crypto
     GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)
add_test(NAME AllTests COMMAND run_tests)
