#pragma once
#include "logger.hpp"

#include <absl/strings/str_cat.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>
#include <grpcpp/support/interceptor.h>
#include <grpcpp/support/server_interceptor.h>

class LoggerInterceptor final : public grpc::experimental::Interceptor {
  public:
    explicit LoggerInterceptor(grpc::experimental::ServerRpcInfo* info)
        : method_(info->method() ? info->method() : "unknown"),
          start_(absl::Now()) {}

    void Intercept(grpc::experimental::InterceptorBatchMethods* methods) override {
        if (method_ == "/grpc.health.v1.Health/Check") {
            methods->Proceed();
            return;
        }

        const grpc::Status* status = methods->GetRecvStatus();
        if (status != nullptr) {
            const auto latency_ms = absl::ToInt64Milliseconds(absl::Now() - start_);
            const auto code = status ? status->error_code() : grpc::StatusCode::UNKNOWN;

            if (code == grpc::StatusCode::OK) {
                SERVICE_VLOG1(absl::StrCat("grpc method=", method_,
                                           " status=OK latency_ms=", latency_ms));
            } else {
                SERVICE_LOG_ERROR(moderation::logging::Subsystem::kGrpc, code,
                    absl::StrCat("method=", method_,
                                 " status_msg=", (status ? status->error_message() : "unknown"),
                                 " latency_ms=", latency_ms));
            }
        }

        methods->Proceed();
    }

  private:
    std::string method_;
    absl::Time  start_;
};

class LoggerInterceptorFactory
    : public grpc::experimental::ServerInterceptorFactoryInterface {
  public:
    grpc::experimental::Interceptor*
    CreateServerInterceptor(grpc::experimental::ServerRpcInfo* info) override {
        return new LoggerInterceptor(info);
    }
};