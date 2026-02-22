#include "config/config.hpp"
#include "handler/handler.hpp"
#include "repository/repository.hpp"
#include "server/server.hpp"
#include "service/service.hpp"
#include <csignal>
#include <iostream>
#include <memory>

std::atomic<bool> shutdown_requested(false);
std::shared_ptr<Server> global_server_ptr;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
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

        std::cout << "Shutting down server...\n";

        kafkaClient->StopConsumer();
        kafkaClient->Flush();
        kafkaClient->Shutdown();

        std::cout << "Server shutdown complete.\n";
    } catch (const std::exception& e) {
        std::cerr << "Main Fatal Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
