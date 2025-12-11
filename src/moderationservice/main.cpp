#include <iostream>
#include "service/service.hpp"
#include "server/server.hpp"
#include "repository/repository.hpp"
#include "handler/handler.hpp"
#include "config/config.hpp"


int main() {
  try
  {
      Config config = Config::New();
      KafkaConfig kafkaConfig = KafkaConfig::New();

      TextProcessingConstants::HashTrieMaps::InitializeForbiddenWords();

      auto repository = std::make_shared<ModerationRepository>(config.database_url);
      auto kafkaClient = std::make_shared<KafkaClient>(kafkaConfig);
      auto service = std::make_shared<ModerationService>(repository, kafkaClient);

      service->InitializeKafkaCallback();
      kafkaClient->StartConsumer();

      auto handler = std::make_shared<ModerationHandler>(service);

      std::unique_ptr<Server> server = std::make_unique<Server>(
        config.host + ":" + config.port, handler, "ModerationService"
      );
      
      server->Start();

      kafkaClient->StopConsumer();
      kafkaClient->Flush();
      kafkaClient->Shutdown();

  }
  catch (const std::exception &e)
  {
      std::cerr << e.what() << std::endl;
      return 1;
  }

  return 0;
}
