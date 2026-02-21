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

// Sizes
constexpr int64_t TEST_LONG_TEXT_LENGTH = 1000;

} // namespace test_constants
