cmake_minimum_required(VERSION 3.9)

project(server VERSION 0.0.1)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED system)

add_library(async SHARED
  async.h
  handler.cpp
  handler.h
  server.h
)

add_executable(server main.cpp)

set_target_properties(async PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  PUBLIC_HEADER async.hpp
)

set_target_properties(server PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(async
  PRIVATE "${CMAKE_BINARY_DIR}" .
)

target_link_libraries(async
	${CMAKE_THREAD_LIBS_INIT} boost_system
)

target_include_directories(server
  PRIVATE "${CMAKE_BINARY_DIR}" .
)

target_link_libraries(server PRIVATE
  async
)

if (MSVC)
  target_compile_options(async PRIVATE
    /W4
  )
  target_compile_options(server PRIVATE
    /W4
  )
else ()
  target_compile_options(async PRIVATE
    -Wall -Wextra -pedantic -lpthread
  )
  target_compile_options(server PRIVATE
    -Wall -Wextra -pedantic -lpthread
  )
endif()

include(GNUInstallDirs)
install(TARGETS server RUNTIME DESTINATION bin)
install(TARGETS async
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT agar10@yandex.ru)
include(CPack)
