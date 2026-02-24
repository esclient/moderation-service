cmake_minimum_required(VERSION 3.20)
project(moderation-service)
set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    add_compile_options(/utf-8)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)
set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH FALSE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_compile_definitions(U_STATIC_IMPLEMENTATION)
endif()

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)

find_package(Protobuf REQUIRED CONFIG)
find_package(gRPC REQUIRED CONFIG)
find_package(ICU REQUIRED COMPONENTS uc i18n data)
find_package(RdKafka REQUIRED)
find_package(Threads REQUIRED)
find_package(libpqxx REQUIRED)
find_package(absl CONFIG REQUIRED)

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
         ${CPM_DOWNLOAD_LOCATION})
endif()

include(${CPM_DOWNLOAD_LOCATION})

cpmaddpackage(NAME tsl-hat-trie GITHUB_REPOSITORY Tessil/hat-trie VERSION 0.6.0)

# Set necessary variables for genproto lib target
set(GENPROTO_LIB "genproto_lib")
set(GENPROTO_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/grpc")
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/.proto/moderation.proto")

# Create output directory for generated files
file(MAKE_DIRECTORY ${GENPROTO_DIR})

# Define generated file paths
set(PROTO_SRCS "${GENPROTO_DIR}/moderation.pb.cc")
set(PROTO_HDRS "${GENPROTO_DIR}/moderation.pb.h")
set(GRPC_SRCS "${GENPROTO_DIR}/moderation.grpc.pb.cc")
set(GRPC_HDRS "${GENPROTO_DIR}/moderation.grpc.pb.h")

# Generate protobuf files using custom command
add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS}
    COMMAND $<TARGET_FILE:protobuf::protoc> ARGS --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/.proto
            --cpp_out=${GENPROTO_DIR} ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating protobuf files")

# Generate gRPC files using custom command
add_custom_command(
    OUTPUT ${GRPC_SRCS} ${GRPC_HDRS}
    COMMAND
        $<TARGET_FILE:protobuf::protoc> ARGS --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/.proto
        --grpc_out=${GENPROTO_DIR} --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
        ${PROTO_FILE}
    DEPENDS ${PROTO_FILE} ${PROTO_HDRS}
    COMMENT "Generating gRPC files")

# Create library from generated sources
add_library(${GENPROTO_LIB} ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS})

target_include_directories(${GENPROTO_LIB}
    PUBLIC
        ${GENPROTO_DIR}
)

target_link_libraries(${GENPROTO_LIB} PUBLIC protobuf::libprotobuf gRPC::grpc++ absl::strings)

# link source files
add_executable(
    ${PROJECT_NAME}
    src/moderationservice/main.cpp
    src/moderationservice/server/server.cpp
    src/moderationservice/service/service.cpp
    src/moderationservice/handler/handler.cpp
    src/moderationservice/repository/repository.cpp
    src/moderationservice/service/text_normalization.cpp
    src/moderationservice/service/leetspeak_normalization.cpp
    src/moderationservice/service/word_checker.cpp
    src/moderationservice/service/text_processor.cpp
    src/moderationservice/model/constants.cpp
    src/moderationservice/kafka/kafkaclient.cpp
    src/moderationservice/model/model_utils.cpp)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE ${GENPROTO_LIB}
            gRPC::grpc++
            gRPC::grpc++_reflection
            protobuf::libprotobuf
            ICU::uc
            ICU::i18n
            ICU::data
            tsl::hat_trie
            Threads::Threads
            RdKafka::rdkafka
            RdKafka::rdkafka++
            libpqxx::pqxx
            absl::strings)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE U_STATIC_IMPLEMENTATION)
endif()

set(PROJECT_SOURCE_DIRS ${PROJECT_SOURCE_DIR}/src/moderationservice ${PROJECT_SOURCE_DIR}/include
                        ${PROJECT_SOURCE_DIR}/tests ${PROJECT_SOURCE_DIR}/benchmark)

file(
    GLOB_RECURSE
    PROJECT_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/src/moderationservice/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/moderationservice/*.cc"
    "${PROJECT_SOURCE_DIR}/src/moderationservice/*.cxx"
    "${PROJECT_SOURCE_DIR}/src/moderationservice/*.h"
    "${PROJECT_SOURCE_DIR}/src/moderationservice/*.hpp"
    "${PROJECT_SOURCE_DIR}/include/*.cpp"
    "${PROJECT_SOURCE_DIR}/include/*.cc"
    "${PROJECT_SOURCE_DIR}/include/*.cxx"
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
    "${PROJECT_SOURCE_DIR}/tests/*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/*.h"
    "${PROJECT_SOURCE_DIR}/tests/*.hpp"
    "${PROJECT_SOURCE_DIR}/benchmark/*.cpp"
    "${PROJECT_SOURCE_DIR}/benchmark/*.h"
    "${PROJECT_SOURCE_DIR}/benchmark/*.hpp")

# ============================================================================
# LINTING SECTION - Temporarily allow system PATH for finding linting tools
# ============================================================================
set(_OLD_CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH ${CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH})
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH TRUE)

option(ENABLE_CLANG_TIDY "Enable clang-tidy analysis during build" OFF)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_BINARY_DIR}")
    else()
        message(STATUS "clang-tidy not found, disabling analysis")
    endif()
endif()

find_program(CPPCHECK_EXE NAMES cppcheck)
if(CPPCHECK_EXE)
    add_custom_target(
        cppcheck
        COMMAND
            ${CPPCHECK_EXE} --enable=warning,style,performance,portability --inline-suppr
            --suppress=*:${CMAKE_BINARY_DIR}/generated/* -I${PROJECT_SOURCE_DIR}/include
            -I${PROJECT_SOURCE_DIR}/src/moderationservice --library=gnu ${PROJECT_SOURCE_DIRS}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running cppcheck"
        VERBATIM)
else()
    message(STATUS "cppcheck not found; 'cppcheck' target will not be created")
endif()

find_program(CPPLINT_EXE NAMES cpplint)
if(CPPLINT_EXE)
    add_custom_target(
        cpplint
        COMMAND ${CPPLINT_EXE} --recursive --filter=-legal/copyright ${PROJECT_SOURCE_DIRS}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running cpplint"
        VERBATIM)
else()
    message(STATUS "cpplint not found; 'cpplint' target will not be created")
endif()

find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
    add_custom_target(
        clang-format
        COMMAND ${CLANG_FORMAT_EXE} -i ${PROJECT_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting sources with clang-format"
        VERBATIM)

    add_custom_target(
        clang-check-format
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${PROJECT_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking clang-format compliance"
        VERBATIM)
else()
    message(STATUS "clang-format not found; 'clang-format' targets will not be created")
endif()

find_program(CMAKE_FORMAT_EXE NAMES cmake-format)
if(CMAKE_FORMAT_EXE)
    set(CMAKE_FILES "${PROJECT_SOURCE_DIR}/CMakeLists.txt")

    add_custom_target(
        cmake-format
        COMMAND ${CMAKE_FORMAT_EXE} -i ${CMAKE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting CMake files with cmake-format"
        VERBATIM)

    add_custom_target(
        cmake-format-check
        COMMAND ${CMAKE_FORMAT_EXE} --check ${CMAKE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking cmake-format compliance"
        VERBATIM)
else()
    message(STATUS "cmake-format not found; 'cmake-format' targets will not be created")
endif()

set(AVAILABLE_LINTERS)
if(CPPCHECK_EXE)
    list(APPEND AVAILABLE_LINTERS "cppcheck")
endif()
if(CPPLINT_EXE)
    list(APPEND AVAILABLE_LINTERS "cpplint")
endif()
if(CLANG_FORMAT_EXE)
    list(APPEND AVAILABLE_LINTERS "clang-format")
endif()

if(CPPCHECK_EXE
   AND CPPLINT_EXE
   AND CLANG_FORMAT_EXE
   AND NOT CPPCHECK_EXE STREQUAL "CPPCHECK_EXE-NOTFOUND"
   AND NOT CPPLINT_EXE STREQUAL "CPPLINT_EXE-NOTFOUND"
   AND NOT CLANG_FORMAT_EXE STREQUAL "CLANG_FORMAT_EXE-NOTFOUND")

    add_custom_target(
        lint-all
        COMMAND ${CMAKE_COMMAND} -E echo "=== Running clang-format check ==="
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${PROJECT_SOURCE_FILES}
        COMMAND ${CMAKE_COMMAND} -E echo "=== Running cppcheck ==="
        COMMAND
            ${CPPCHECK_EXE} --enable=warning,style,performance,portability --inline-suppr
            --suppress=*:${CMAKE_BINARY_DIR}/generated/* --suppress=*:${CMAKE_BINARY_DIR}/_deps/*
            -I${PROJECT_SOURCE_DIR}/include -I${PROJECT_SOURCE_DIR}/src/moderationservice
            --library=gnu ${PROJECT_SOURCE_DIR}/src/moderationservice ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/tests ${PROJECT_SOURCE_DIR}/benchmark
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "=== Running cpplint ==="
        COMMAND
            ${CPPLINT_EXE} --recursive
            --filter=-legal/copyright,-whitespace/indent,-whitespace/comments,-build/include_order,-build/include_subdir
            --linelength=120 ${PROJECT_SOURCE_DIR}/src/moderationservice
            ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/tests
            ${PROJECT_SOURCE_DIR}/benchmark
        COMMAND ${CMAKE_COMMAND} -E echo ""
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running all linters"
        VERBATIM)
    message(STATUS "Created 'lint-all' target with: ${AVAILABLE_LINTERS}")
else()
    message(
        STATUS
            "Not all linters found (available: ${AVAILABLE_LINTERS}); 'lint-all' target will not be created"
    )
endif()

# Restore the original setting to keep Conan dependencies isolated
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH ${_OLD_CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH})
# ============================================================================
# END LINTING SECTION
# ============================================================================

# link header files
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/src/moderationservice
            ${PROJECT_SOURCE_DIR}/src/moderationservice/repository
            ${PROJECT_SOURCE_DIR}/src/moderationservice/handler
            ${PROJECT_SOURCE_DIR}/src/moderationservice/service
            ${PROJECT_SOURCE_DIR}/src/moderationservice/model
            ${PROJECT_SOURCE_DIR}/src/moderationservice/kafka
            ${GENPROTO_DIR})

add_subdirectory(tests)

option(BUILD_BENCHMARKS "Build benchmark executables" OFF)
if(BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()
