find_package(benchmark REQUIRED CONFIG)

add_executable(
    text_processing_benchmark
    performance/text_processing_benchmark.cpp
    ${PROJECT_SOURCE_DIR}/src/moderationservice/service/text_processor.cpp
    ${PROJECT_SOURCE_DIR}/src/moderationservice/service/text_normalization.cpp
    ${PROJECT_SOURCE_DIR}/src/moderationservice/service/leetspeak_normalization.cpp
    ${PROJECT_SOURCE_DIR}/src/moderationservice/service/word_checker.cpp
    ${PROJECT_SOURCE_DIR}/src/moderationservice/model/constants.cpp
)

target_include_directories(
    text_processing_benchmark
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/moderationservice
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/generated/grpc
)

target_link_libraries(
    text_processing_benchmark
    PRIVATE
        benchmark::benchmark
        ${GENPROTO_LIB}
        ICU::uc
        ICU::i18n
        ICU::data
        tsl::hat_trie
)

