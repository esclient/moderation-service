cmake_minimum_required(VERSION 3.20)
project(moderation-service)

set(CPM_DOWNLOAD_VERSION 0.40.2)
if(CPM_SOURCE_CACHE)
  set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
  set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  file(DOWNLOAD
       https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
       ${CPM_DOWNLOAD_LOCATION}
  )
endif()

include(${CPM_DOWNLOAD_LOCATION})

CPMAddPackage(
  NAME gRPC
  GITHUB_REPOSITORY grpc/grpc
  GIT_TAG v1.76.0
  OPTIONS
    "gRPC_BUILD_TESTS OFF"
    "gRPC_BUILD_CODEGEN ON"
    "gRPC_BUILD_CSHARP_EXT OFF"
)

CPMAddPackage(
  NAME ICU
  GITHUB_REPOSITORY unicode-org/icu
  GIT_TAG release-74-2
  SOURCE_SUBDIR icu4c/source
  OPTIONS
    "ICU_BUILD_TESTS OFF"
)

CPMAddPackage(
  NAME tsl-hat-trie
  GITHUB_REPOSITORY Tessil/hat-trie
  VERSION 0.6.0  
)

# set necessary variables for genproto lib target
set(GENPROTO_LIB "genproto_lib")
set(GENPROTO_DIR "${PROJECT_SOURCE_DIR}/src/moderationservice/grpc")

# loop through genproto dir
file(GLOB_RECURSE PROTO_SRC "${GENPROTO_DIR}/*.pb.cc")
file(GLOB_RECURSE PROTO_HDR "${GENPROTO_DIR}/*.pb.h")

add_library(${GENPROTO_LIB} ${PROTO_SRC} ${PROTO_HDR})

target_link_libraries(${GENPROTO_LIB}
    PUBLIC
        grpc++
        protobuf::libprotobuf
)

target_include_directories(${GENPROTO_LIB}
    PUBLIC
        ${GENPROTO_DIR}
)

#link source files
add_executable(${PROJECT_NAME}
    src/moderationservice/main.cpp
    src/moderationservice/server/server.cpp
    src/moderationservice/service/service.cpp
    src/moderationservice/service/text_normalization.cpp
    src/moderationservice/service/leetspeak_normalization.cpp
    src/moderationservice/service/word_checker.cpp
    src/moderationservice/service/text_processor.cpp
    src/moderationservice/model/constants.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${GENPROTO_LIB}
        grpc++_reflection
        icuuc
        icui18n  
        icudata
        icuio
        icutu
        tsl::hat_trie
)

#link header files
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/moderationservice/service 
        ${PROJECT_SOURCE_DIR}/src/moderationservice/model
)
