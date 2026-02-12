cmake_minimum_required(VERSION 3.20)
enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(TEST_DEPS genproto_lib Threads::Threads)

find_package(Protobuf REQUIRED CONFIG)
find_package(gRPC REQUIRED CONFIG)
find_package(ICU REQUIRED COMPONENTS uc i18n data)
find_package(RdKafka REQUIRED)
find_package(Threads REQUIRED)
find_package(libpqxx REQUIRED)


add_executable(fake_repository_test unit/repository/repository_test.cpp
               mocks/fake_moderation_repository.cpp)
target_include_directories(fake_repository_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/moderationservice
    ${CMAKE_SOURCE_DIR}/src/moderationservice/repository
    ${CMAKE_SOURCE_DIR}/tests
    ${GENPROTO_DIR})
target_link_libraries(fake_repository_test PRIVATE GTest::gtest_main ${TEST_DEPS})
add_test(NAME FakeRepositoryTest COMMAND fake_repository_test)

PRIVATE ${CMAKE_SOURCE_DIR}/src/moderationservice ${CMAKE_SOURCE_DIR}/include ${GENPROTO_DIR}

set(SERVICE_TEST_SOURCES
    src/moderationservice/service/text_normalization.cpp
    src/moderationservice/service/leetspeak_normalization.cpp
    src/moderationservice/service/word_checker.cpp
    src/moderationservice/service/text_processor.cpp
    src/moderationservice/model/constants.cpp
)

add_executable(text_normalization_test unit/service/text_normalization_test.cpp)