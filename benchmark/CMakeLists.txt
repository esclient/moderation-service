find_package(benchmark REQUIRED CONFIG)

add_executable(
    text_processing_benchmark
    performance/text_processing_benchmark.cpp
)
target_link_libraries(
    text_processing_benchmark
    PRIVATE
        benchmark::benchmark
        moderation_core
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

