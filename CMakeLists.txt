cmake_minimum_required(VERSION 3.15)
project(moderation-service VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Make an "include" directory available for optional headers (tinylog fallback)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Find Protobuf (required for generated .pb.cc/.h)
find_package(Protobuf REQUIRED)
message(STATUS "Protobuf include: ${Protobuf_INCLUDE_DIRS}")

# Try to find gRPC (prefer CONFIG, then fall back to classic find)
find_package(gRPC CONFIG QUIET)
if(NOT gRPC_FOUND)
  # Try classic find_package; on some systems this won't provide a config.
  find_package(gRPC QUIET)
endif()

if(NOT gRPC_FOUND)
  message(STATUS "gRPC CMake config not found; attempting pkg-config fallback")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GRPC_PKG grpc++ grpc)
  if(GRPC_PKG_FOUND)
    message(STATUS "Found gRPC via pkg-config: ${GRPC_PKG_INCLUDEDIR}")
    # Add include dirs and libraries discovered by pkg-config
    target_include_directories(${PROJECT_NAME} PRIVATE ${GRPC_PKG_INCLUDEDIR})
    foreach(lib ${GRPC_PKG_LIBRARIES})
      target_link_libraries(${PROJECT_NAME} PRIVATE ${lib})
    endforeach()
    set(gRPC_FOUND TRUE)
  else()
    message(STATUS "gRPC not found via pkg-config either; attempting header/lib search fallback")
    # Try to locate headers and libraries directly (useful when installed without pkg-config/CMake config)
    find_path(GRPC_INCLUDE_DIR grpcpp/server.h HINTS /usr/local/include /usr/include ENV GRPC_ROOT)
    find_library(GRPCPP_LIBRARY NAMES grpc++ grpc++_unsecure grpc++)
    find_library(GRPC_LIBRARY NAMES grpc)
    if(GRPC_INCLUDE_DIR)
      message(STATUS "Found gRPC headers in ${GRPC_INCLUDE_DIR}")
      target_include_directories(${PROJECT_NAME} PRIVATE ${GRPC_INCLUDE_DIR})
    endif()
    if(GRPCPP_LIBRARY)
      message(STATUS "Found gRPC++ lib: ${GRPCPP_LIBRARY}")
      target_link_libraries(${PROJECT_NAME} PRIVATE ${GRPCPP_LIBRARY})
    endif()
    if(GRPC_LIBRARY)
      message(STATUS "Found gRPC lib: ${GRPC_LIBRARY}")
      target_link_libraries(${PROJECT_NAME} PRIVATE ${GRPC_LIBRARY})
    endif()
    if(NOT GRPC_INCLUDE_DIR OR (NOT GRPCPP_LIBRARY AND NOT GRPC_LIBRARY))
      message(STATUS "gRPC not found via header/lib search either; you may need to install gRPC or set gRPC_DIR/CMAKE_PREFIX_PATH or PKG_CONFIG_PATH")
    else()
      set(gRPC_FOUND TRUE)
    endif()
  endif()
endif()
message(STATUS "gRPC found: ${gRPC_FOUND}")

find_package(Threads REQUIRED)

# Collect all .cpp source files under src/
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp"
)

if(PROJECT_SOURCES)
  list(REMOVE_DUPLICATES PROJECT_SOURCES)
else()
  message(FATAL_ERROR "No source files found under src/. Did you run this from the repository root?")
endif()

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
  ${Protobuf_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link with protobuf and gRPC
if(TARGET protobuf::libprotobuf)
  target_link_libraries(${PROJECT_NAME} PRIVATE protobuf::libprotobuf)
elseif(PROTOBUF_LIBRARY)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${PROTOBUF_LIBRARY})
endif()

if(TARGET gRPC::grpc++)
  target_link_libraries(${PROJECT_NAME} PRIVATE gRPC::grpc++)
elseif(TARGET grpc++)
  target_link_libraries(${PROJECT_NAME} PRIVATE grpc++)
else()
  # Best-effort: some systems provide gRPC as gRPC::grpc++ or grpc++
  find_library(GRPC_LIB grpc++)
  if(GRPC_LIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${GRPC_LIB})
  endif()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

# Compiler warning flags
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")

# tinylog detection: prefer user-installed; fall back to bundled header in include/
find_path(TINYLOG_INCLUDE_DIR tinylog.hpp HINTS ${CMAKE_CURRENT_SOURCE_DIR}/include ENV TINYLOG_ROOT)
if(TINYLOG_INCLUDE_DIR)
  message(STATUS "Found tinylog include: ${TINYLOG_INCLUDE_DIR}")
  target_include_directories(${PROJECT_NAME} PRIVATE ${TINYLOG_INCLUDE_DIR})
else()
  message(STATUS "tinylog.hpp not found system-wide - using bundled fallback header in ${CMAKE_CURRENT_SOURCE_DIR}/include/")
  target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

# Helpful summary
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Sources (${PROJECT_SOURCES})")
