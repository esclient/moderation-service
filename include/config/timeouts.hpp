#pragma once

#include <cstdint>

namespace moderation::config {

// Kafka timeouts in milliseconds
constexpr int32_t KAFKA_FLUSH_TIMEOUT_MS = 10000;
constexpr int32_t KAFKA_CONSUME_TIMEOUT_MS = 1000;

} // namespace moderation::config
