#include "moderationservice/model/constants.hpp"
#include "moderationservice/service/text_processor.hpp"
#include <benchmark/benchmark.h>
#include <string>


static void BM_TextProcessing_Short(benchmark::State& state) {
    TextProcessingConstants::HashTrieMaps::InitializeForbiddenWords();
    std::string text(50, 'a');

    for (auto _ : state) {
        benchmark::DoNotOptimize(TextProcessor::TextProcessing(text));
    }
}

static void BM_TextProcessing_Medium(benchmark::State& state) {
    TextProcessingConstants::HashTrieMaps::InitializeForbiddenWords();
    std::string text(1024, 'a');
    for (auto _ : state) {
        benchmark::DoNotOptimize(TextProcessor::TextProcessing(text));
    }
}

BENCHMARK(BM_TextProcessing_Short);
BENCHMARK(BM_TextProcessing_Medium);
BENCHMARK_MAIN();