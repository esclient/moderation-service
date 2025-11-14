#include <iostream>
#include "service/service.hpp"
#include "server/server.hpp"
#include "config/config.hpp"


int main() {
  try
  {
      Config config = Config::New();
      KafkaConfig kafkaConfig = KafkaConfig::New();

      TextProcessingConstants::HashTrieMaps::InitializeForbiddenWords();

      auto kafkaClient = std::make_shared<KafkaClient>(kafkaConfig);

      std::shared_ptr<ModerationService> mService = std::make_shared<ModerationService>(kafkaClient);
      
      mService->InitializeKafkaCallback();
      kafkaClient->StartConsumer();

      std::unique_ptr<Server> server = std::make_unique<Server>(config.host + ":" + config.port, mService, "ModerationService");
      
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
