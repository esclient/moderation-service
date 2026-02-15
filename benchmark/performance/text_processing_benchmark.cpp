#include "model/constants.hpp"
#include "service/text_processor.hpp"
#include <benchmark/benchmark.h>
#include <string>

static bool initialized = false;

static void InitOnce() {
    if (!initialized) {
        TextProcessingConstants::HashTrieMaps::InitializeForbiddenWords();
        initialized = true;
    }
}

static void BM_TextProcessing_Short(benchmark::State& state) {
    InitOnce();
    std::string text(50, 'a');

    for (auto _ : state) {
        benchmark::DoNotOptimize(TextProcessor::TextProcessing(text));
    }
}

static void BM_TextProcessing_Medium(benchmark::State& state) {
    InitOnce();
    std::string text(1024, 'a');
    for (auto _ : state) {
        benchmark::DoNotOptimize(TextProcessor::TextProcessing(text));
    }
}

static void BM_TextProcessing_Realistic(benchmark::State& state) {
    InitOnce();
    std::string text = "Hey everyone! Check out this awesome product at example.com 🔥🔥🔥";
    for (auto _ : state) {
        benchmark::DoNotOptimize(TextProcessor::TextProcessing(text));
    }
}

static void BM_TextProcessing_WithProfanity(benchmark::State& state) {
    InitOnce();
    std::string text = "This is a test with some b@d w0rds and leetspeak";
    for (auto _ : state) {
        benchmark::DoNotOptimize(TextProcessor::TextProcessing(text));
    }
}

static void BM_TextProcessing_Long(benchmark::State& state) {
    InitOnce();
    std::string text(5000, 'a'); // 5KB
    for (auto _ : state) {
        benchmark::DoNotOptimize(TextProcessor::TextProcessing(text));
    }
}

static void BM_TextProcessing_Extreme_Long(benchmark::State& state) {
    InitOnce();
    std::string text(100000, 'A');
    for (auto _ : state) {
        benchmark::DoNotOptimize(TextProcessor::TextProcessing(text));
    }
}

static void BM_InitializeForbiddenWords(benchmark::State& state) {
    for (auto _ : state) {
        TextProcessingConstants::HashTrieMaps::InitializeForbiddenWords();
    }
}

static void BM_TextProcessing_Complexity(benchmark::State& state) {
    InitOnce();
    const size_t size = state.range(0);
    std::string text(size, 'a');

    for (auto _ : state) {
        benchmark::DoNotOptimize(TextProcessor::TextProcessing(text));
    }

    state.SetComplexityN(size);
}

BENCHMARK(BM_TextProcessing_Short);
BENCHMARK(BM_TextProcessing_Medium);
BENCHMARK(BM_TextProcessing_Realistic);
BENCHMARK(BM_TextProcessing_WithProfanity);
BENCHMARK(BM_TextProcessing_Long);
BENCHMARK(BM_TextProcessing_Extreme_Long);
BENCHMARK(BM_InitializeForbiddenWords);
BENCHMARK(BM_TextProcessing_Complexity)
    ->RangeMultiplier(2)
    ->Range(32, 131072)
    ->Complexity(benchmark::oN);
BENCHMARK_MAIN();