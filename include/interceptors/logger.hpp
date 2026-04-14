#pragma once

#include <absl/log/log.h>
#include <absl/strings/ascii.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/string_view.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>

#include <grpcpp/support/status.h>
#include <librdkafka/rdkafkacpp.h>
#include <unicode/utypes.h>
#include <string>

namespace moderation::logging {

enum class Subsystem {
    kGrpc,
    kKafka,
    kIcu,
    kAlgorithm,
    kRepository,
    kService,
    kServer,
    kConfig,
    kUnknown
};

inline absl::TimeZone GetLocalTZ() {
    static absl::TimeZone timezone = [] {
        absl::TimeZone time;
        absl::LoadTimeZone("Europe/Berlin", &time);
        return time;
    }();
    return timezone;
}

inline absl::string_view SubsystemName(Subsystem systemName) {
    switch (systemName) {
        case Subsystem::kGrpc: return "grpc";
        case Subsystem::kKafka: return "kafka";
        case Subsystem::kIcu: return "icu";
        case Subsystem::kAlgorithm: return "algorithm";
        case Subsystem::kRepository: return "repository";
        case Subsystem::kService: return "service";
        case Subsystem::kServer: return "server";
        case Subsystem::kConfig: return "config";
        default: return "unknown";
    }
}

// --- explanation mapping per subsystem ---

inline absl::string_view Explain(Subsystem, grpc::StatusCode code) {
    switch (code) {
        case grpc::StatusCode::DEADLINE_EXCEEDED:
            return "client or server timeout, check network or increase deadline";
        case grpc::StatusCode::UNAVAILABLE:
            return "service unavailable, check endpoint health and connectivity";
        case grpc::StatusCode::INVALID_ARGUMENT:
            return "request validation failed, verify input fields";
        default:
            return "grpc request failed, inspect status code and service logs";
    }
}

inline absl::string_view Explain(Subsystem, grpc::Status status) {
    return Explain(Subsystem::kGrpc, status.error_code());
}

inline absl::string_view Explain(Subsystem, RdKafka::ErrorCode code) {
    switch (code) {
        case RdKafka::ERR__TRANSPORT:
            return "broker unreachable, check connectivity and bootstrap config";
        case RdKafka::ERR__TIMED_OUT:
            return "operation timed out, check broker responsiveness and timeouts";
        case RdKafka::ERR__AUTHENTICATION:
            return "authentication failed, verify SASL/SSL credentials";
        default:
            return "kafka operation failed, inspect broker/client configuration";
    }
}

inline absl::string_view Explain(Subsystem, UErrorCode code) {
    switch (code) {
        case U_BUFFER_OVERFLOW_ERROR:
            return "output buffer too small for the converted string";
        case U_INVALID_CHAR_FOUND:
            return "invalid unicode sequence found in input data";
        case U_MEMORY_ALLOCATION_ERROR:
            return "memory allocation failed during unicode operation";
        default:
            return "ICU operation failed, verify unicode input and ICU configuration";
    }
}

// fallback for custom codes

inline absl::string_view Explain(Subsystem subsystem, absl::string_view code) {
    if (subsystem == Subsystem::kAlgorithm) {
        if (code == "TOKENIZE_FAIL") return "tokenization failed, input normalization may be invalid";
        if (code == "NORMALIZE_FAIL") return "normalization failed, inspect text and ICU preprocessing";
    }
    return "operation failed, inspect subsystem-specific context";
}

inline absl::string_view GrpcCodeName(grpc::StatusCode code) {
    switch (code) {
        case grpc::StatusCode::OK: return "OK";
        case grpc::StatusCode::CANCELLED: return "CANCELLED";
        case grpc::StatusCode::UNKNOWN: return "UNKNOWN";
        case grpc::StatusCode::INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case grpc::StatusCode::DEADLINE_EXCEEDED: return "DEADLINE_EXCEEDED";
        case grpc::StatusCode::NOT_FOUND: return "NOT_FOUND";
        case grpc::StatusCode::ALREADY_EXISTS: return "ALREADY_EXISTS";
        case grpc::StatusCode::PERMISSION_DENIED: return "PERMISSION_DENIED";
        case grpc::StatusCode::RESOURCE_EXHAUSTED: return "RESOURCE_EXHAUSTED";
        case grpc::StatusCode::FAILED_PRECONDITION: return "FAILED_PRECONDITION";
        case grpc::StatusCode::ABORTED: return "ABORTED";
        case grpc::StatusCode::OUT_OF_RANGE: return "OUT_OF_RANGE";
        case grpc::StatusCode::UNIMPLEMENTED: return "UNIMPLEMENTED";
        case grpc::StatusCode::INTERNAL: return "INTERNAL";
        case grpc::StatusCode::UNAVAILABLE: return "UNAVAILABLE";
        case grpc::StatusCode::DATA_LOSS: return "DATA_LOSS";
        case grpc::StatusCode::UNAUTHENTICATED: return "UNAUTHENTICATED";
        default: return "UNRECOGNIZED";
    }
}

inline std::string FormatError(Subsystem subsystem, grpc::StatusCode code, absl::string_view message) {
    const auto timespan = absl::FormatTime("%Y-%m-%dT%H:%M:%E3S%Ez", absl::Now(), GetLocalTZ());
    return absl::StrCat(
        "timespan=", timespan,
        " subsystem=", SubsystemName(subsystem),
        " code=", GrpcCodeName(code),
        " message=\"", message, "\"",
        " explanation=\"", Explain(subsystem, code), "\"");
}

inline std::string FormatError(Subsystem subsystem, grpc::Status status, absl::string_view message) {
    return FormatError(subsystem, status.error_code(),
                       absl::StrCat(message, " status_message=\"", status.error_message(), "\""));
}

inline std::string FormatError(Subsystem subsystem, RdKafka::ErrorCode code, absl::string_view message) {
    const auto timespan = absl::FormatTime("%Y-%m-%dT%H:%M:%E3S%Ez", absl::Now(), GetLocalTZ());
    return absl::StrCat(
        "timespan=", timespan,
        " subsystem=", SubsystemName(subsystem),
        " code=", RdKafka::err2str(code),
        " message=\"", message, "\"",
        " explanation=\"", Explain(subsystem, code), "\"");
}

inline std::string FormatError(Subsystem subsystem, UErrorCode code, absl::string_view message) {
    const auto timespan = absl::FormatTime("%Y-%m-%dT%H:%M:%E3S%Ez", absl::Now(), GetLocalTZ());
    return absl::StrCat(
        "timespan=", timespan,
        " subsystem=", SubsystemName(subsystem),
        " code=", u_errorName(code),
        " message=\"", message, "\"",
        " explanation=\"", Explain(subsystem, code), "\"");
}

inline std::string FormatError(Subsystem subsystem, absl::string_view code, absl::string_view message) {
    const auto timespan = absl::FormatTime("%Y-%m-%dT%H:%M:%E3S%Ez", absl::Now(), GetLocalTZ());
    return absl::StrCat(
        "timespan=", timespan,
        " subsystem=", SubsystemName(subsystem),
        " code=", code,
        " message=\"", message, "\"",
        " explanation=\"", Explain(subsystem, code), "\"");
}

inline std::string FormatError(Subsystem subsystem, const std::string& code,
                               absl::string_view message) {
    return FormatError(subsystem, absl::string_view(code), message);
}

inline std::string FormatError(Subsystem subsystem, const char* code, absl::string_view message) {
    return FormatError(subsystem, absl::string_view(code ? code : "UNKNOWN"), message);
}

}  // namespace moderation::logging

#define SERVICE_LOG_ERROR(subsystem, code, message) \
    LOG(ERROR).AtLocation(__FILE__, __LINE__) << ::moderation::logging::FormatError((subsystem), (code), (message))

#define SERVICE_LOG_INFO(message) LOG(INFO).AtLocation(__FILE__, __LINE__) << (message)
#define SERVICE_VLOG1(message) VLOG(1) << (message)