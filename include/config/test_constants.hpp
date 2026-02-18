#pragma once
#include <cstdint>

namespace test_constants {

// Test fixture configuration
constexpr int32_t TEST_RETRY_BACKOFF_MS = 100;

// Test IDs
constexpr int64_t TEST_ID_BASIC = 42;
constexpr int64_t TEST_ID_SECONDARY = 99;
constexpr int64_t TEST_ID_HANDLER = 123;
constexpr int64_t TEST_ID_LONG_TEXT = 456;
constexpr int64_t TEST_ID_SPECIAL = 789;
constexpr int64_t TEST_ID_MAX = 999;

// Moderation scores used in service tests
constexpr int64_t TEST_SCORE_FLAGGED = 99;
constexpr int64_t TEST_SCORE_CLEAN = 55;
constexpr int64_t TEST_SCORE_BAD_CONTENT = 777;

// Sizes
constexpr int64_t TEST_LONG_TEXT_LENGTH = 1000;

} // namespace test_constants