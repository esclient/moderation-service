#include "config/config.hpp"
#include "handler/handler.hpp"
#include "interceptors/logger.hpp"
#include "repository/repository.hpp"
#include "server/server.hpp"
#include "service/service.hpp"
#include <csignal>
#include <memory>

std::atomic<bool> shutdown_requested(false);
std::shared_ptr<Server> global_server_ptr;

void signalHandler(int signum) {
    SERVICE_LOG_INFO(absl::StrCat("interrupt signal received signal=", signum));
    shutdown_requested.store(true);

    if (global_server_ptr) {
        global_server_ptr->Stop();
    }
}

int main() {
    try {
        std::signal(SIGINT, signalHandler);  // Handle Ctrl+C
        std::signal(SIGTERM, signalHandler); // Docker/Kubernetes termination signal

        Config config = Config::New();
        KafkaConfig kafkaConfig = KafkaConfig::New();

        TextProcessingConstants::HashTrieMaps::InitializeForbiddenWords();

        auto repository = std::make_shared<ModerationRepository>(config.database_url);
        auto kafkaClient = std::make_shared<KafkaClient>(kafkaConfig);
        auto service = std::make_shared<ModerationService>(repository, kafkaClient);

        service->InitializeKafkaCallback();
        kafkaClient->StartConsumer();

        auto handler = std::make_shared<ModerationHandler>(service);

        auto server =
            std::make_shared<Server>(config.host + ":" + config.port, handler, "ModerationService");

        global_server_ptr = server;

        server->Start();

        while (!shutdown_requested.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        SERVICE_LOG_INFO("shutting down server");

        kafkaClient->StopConsumer();
        kafkaClient->Flush();
        kafkaClient->Shutdown();

        SERVICE_LOG_INFO("server shutdown complete");
    } catch (const std::exception& e) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kServer, "MAIN_FATAL_ERROR", e.what());
        return 1;
    }

    return 0;
}
