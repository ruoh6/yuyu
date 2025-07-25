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
#include_directories(/apps/sylar/include)
include_directories(/usr/local/include)
include_directories(/usr/local/ssl/include)
#link_directories(/apps/sylar/lib)
link_directories(/usr/local/lib)
# no lib64
#link_directories(/apps/sylar/lib64)

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
#     include_directories(${ZLIB_INCLUDE_DIRS})
# endif()

 find_package(yaml-cpp REQUIRED)
 if(YAMLCPP_FOUND)
     include_directories(${YAMLCPP_INCLUDE_DIRS})
 endif()
set(LIB_SRC
    yuyu/log.cc
    yuyu/util.cc
    yuyu/config.cc
    yuyu/thread.cc
    yuyu/fiber.cc
    )

 set(LIBS
     yuyu
#     dl
     pthread
     yaml-cpp
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


yuyu_add_executable(test_log "tests/test_log.cc" yuyu "${LIBS}")
yuyu_add_executable(test_config "tests/test_config.cc" yuyu "${LIBS}")
yuyu_add_executable(test_thread "tests/test_thread.cc" yuyu "${LIBS}")
yuyu_add_executable(test_util "tests/test_util.cc" yuyu "${LIBS}")
yuyu_add_executable(test_fiber "tests/test_fiber.cc" yuyu "${LIBS}")
#add_executable(test_log ${PROJECT_SOURCE_DIR}/tests/test_log.cc)
#add_dependencies(test_log yuyu)
#target_link_libraries(test_log yuyu)

#add_executable(test_config ${PROJECT_SOURCE_DIR}/tests/test_config.cc)
#add_dependencies(test_config yuyu)
#target_link_libraries(test_config yuyu ${YAMLCPP_LIBRARIES})
