#include <iostream>
#include "service/service.hpp"
#include "server/server.hpp"
#include "config/config.hpp"

int main() {
  try
  {
      Config config = Config::New(); 
      std::shared_ptr<ModerationService> mService = std::make_shared<ModerationService>();
      std::unique_ptr<Server> server = std::make_unique<Server>(config.host + ":" + config.port, mService, "ModerationService");

      server->Start();
  }
  catch (const std::exception &e)
  {
      std::cerr << e.what() << std::endl;
      return 1;
  }

  return 0;
}
