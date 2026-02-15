cmake_minimum_required(VERSION 3.20)
enable_testing()
include(GoogleTest)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(TEST_DEPS 
    genproto_lib 
    Threads::Threads
)

set(SERVICE_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/moderationservice/service/text_normalization.cpp
    ${CMAKE_SOURCE_DIR}/src/moderationservice/service/leetspeak_normalization.cpp
    ${CMAKE_SOURCE_DIR}/src/moderationservice/service/word_checker.cpp
    ${CMAKE_SOURCE_DIR}/src/moderationservice/service/text_processor.cpp
    ${CMAKE_SOURCE_DIR}/src/moderationservice/model/constants.cpp
)

function(add_service_test TEST_NAME)
    add_executable(${TEST_NAME} ${ARGN} ${SERVICE_TEST_SOURCES})

    target_include_directories(${TEST_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/moderationservice 
    ${CMAKE_SOURCE_DIR}/include 
    ${CMAKE_SOURCE_DIR}/tests
    ${GENPROTO_DIR}
    )

    target_link_libraries(${TEST_NAME} PRIVATE 
        GTest::gtest_main 
        ${GENPROTO_LIB} 
        ICU::uc ICU::i18n 
        ICU::data 
        tsl::hat_trie
        libpqxx::pqxx
        RdKafka::rdkafka
        RdKafka::rdkafka++
        ${TEST_DEPS}
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# ============================================================================
# Tests
# ============================================================================
add_service_test(fake_repository_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/repository/repository_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/fake_moderation_repository.cpp
)
gtest_discover_tests(fake_repository_test)

add_service_test(text_normalization_test
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/service/text_normalization_test.cpp
)
gtest_discover_tests(text_normalization_test)

add_service_test(word_checker_test
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/service/word_checker_test.cpp
)
gtest_discover_tests(word_checker_test)

add_service_test(text_processor_test
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/service/text_processor_test.cpp
)
gtest_discover_tests(text_processor_test)

add_service_test(moderation_service_test
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/service/moderation_service_test.cpp
    ${CMAKE_SOURCE_DIR}/src/moderationservice/service/service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/fake_moderation_repository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/mock_kafka_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/test_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/moderationservice/kafka/kafkaclient.cpp 
)
gtest_discover_tests(moderation_service_test)

add_service_test(handler_test
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/handler/handler_test.cpp
    ${CMAKE_SOURCE_DIR}/src/moderationservice/handler/handler.cpp
    ${CMAKE_SOURCE_DIR}/src/moderationservice/service/service.cpp
    ${CMAKE_SOURCE_DIR}/src/moderationservice/kafka/kafkaclient.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/fake_moderation_repository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/mock_kafka_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/test_utils.cpp
)
gtest_discover_tests(handler_test)
