cmake_minimum_required(VERSION 3.20)
project(moderation-service)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc i18n)
#find_package(utf8_range REQUIRED)

# set necessary variables for genproto lib target
set(GENPROTO_LIB "genproto_lib")
set(GENPROTO_DIR "${PROJECT_SOURCE_DIR}/src/moderationservice/grpc")

# loop through genproto dir
file(GLOB_RECURSE PROTO_SRC "${GENPROTO_DIR}/*.pb.cc")
file(GLOB_RECURSE PROTO_HDR "${GENPROTO_DIR}/*.pb.h")

add_library(${GENPROTO_LIB} ${PROTO_SRC} ${PROTO_HDR})

target_link_libraries(${GENPROTO_LIB}
    PUBLIC
        gRPC::grpc++
        protobuf::libprotobuf
        #utf8_range::utf8_range
        #utf8_range::utf8_validity
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
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${GENPROTO_LIB}
        gRPC::grpc++_reflection
        ICU::uc 
        ICU::i18n
)

#link header files
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${PROJECT_SOURCE_DIR}/external/tsl-hat-trie/include
)
