#pragma once
#include <absl/log/log_sink.h>
#include <absl/log/log_sink_registry.h>
#include <absl/log/log_entry.h>
#include <absl/log/initialize.h>
#include <absl/log/globals.h>
#include <iostream>

namespace moderation::logging {

class ReadableLogSink : public absl::LogSink {
public:
    std::atomic<bool> installed{false};

    void Send(const absl::LogEntry& entry) override {
        if (!installed) return; 
        
        std::string_view level;
        switch (entry.log_severity()) {
            case absl::LogSeverity::kInfo:    level = "INFO";    break;
            case absl::LogSeverity::kWarning: level = "WARNING"; break;
            case absl::LogSeverity::kError:   level = "ERROR";   break;
            case absl::LogSeverity::kFatal:   level = "FATAL";   break;
            default:                          level = "UNKNOWN";  break;
        }
        std::cerr
            << "[" << level << "] "
            << entry.source_basename() << ":" << entry.source_line() << " "
            << entry.text_message() << "\n";
    }

    void Flush() override { std::cerr.flush(); }
};

inline ReadableLogSink* GetSink() {
    static ReadableLogSink sink;
    return &sink;
}

inline void InstallReadableSink() {
    absl::AddLogSink(GetSink()); 
}

inline void ActivateReadableSink() {
    absl::InitializeLog();
    absl::SetStderrThreshold(absl::LogSeverityAtLeast::kInfinity);
    GetSink()->installed = true;
}

} // namespace moderation::logging