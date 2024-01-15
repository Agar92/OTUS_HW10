cmake_minimum_required(VERSION 3.15)

project(bulk_server VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost COMPONENTS system REQUIRED)
find_package(Threads)
include_directories(${Boost_INCLUDE_DIR})

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories(${CMAKE_BINARY_DIR})

add_library(threadpool src/threadpool.cpp include/threadpool.h)
set_property(TARGET threadpool PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(logger src/logger.cpp include/logger.h)
set_property(TARGET logger PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(command src/processor.cpp src/commands.cpp
        include/processor.h include/commands.h src/main.cpp src/server.cpp)
set_property(TARGET command PROPERTY POSITION_INDEPENDENT_CODE ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_link_libraries(logger pthread)
    target_link_libraries(threadpool pthread)
endif ()

add_library(async SHARED src/async.cpp include/async.h)
set_property(TARGET async PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(async command logger threadpool)

add_executable(bulk_server src/main.cpp src/server.cpp include/server.h)

target_link_libraries(bulk_server
        async
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS async LIBRARY DESTINATION lib)
install(TARGETS bulk_server RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT agar10@yandex.ru)
include(CPack)
