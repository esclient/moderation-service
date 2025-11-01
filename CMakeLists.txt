cmake_minimum_required(VERSION 3.20)
project(moderation-service)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)
set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH FALSE)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

find_package(gRPC CONFIG REQUIRED PATHS ${CMAKE_BINARY_DIR})
find_package(ICU REQUIRED COMPONENTS uc i18n data)
find_package(Threads REQUIRED)

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
  NAME tsl-hat-trie
  GITHUB_REPOSITORY Tessil/hat-trie
  VERSION 0.6.0  
)

# set necessary variables for genproto lib target
set(GENPROTO_LIB "genproto_lib")
set(GENPROTO_DIR "${PROJECT_SOURCE_DIR}/src/moderationservice/grpc")

# Explicitly list proto files
add_library(${GENPROTO_LIB}
    ${GENPROTO_DIR}/moderation.pb.cc
    ${GENPROTO_DIR}/moderation.grpc.pb.cc
)

target_link_libraries(${GENPROTO_LIB}
    PUBLIC
        gRPC::grpc++
        protobuf::libprotobuf
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
        gRPC::grpc++_reflection
        ICU::uc
        ICU::i18n  
        ICU::data
        tsl::hat_trie
        Threads::Threads
)

#link header files
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/moderationservice/service 
        ${PROJECT_SOURCE_DIR}/src/moderationservice/model
        ${GENPROTO_DIR}
)
