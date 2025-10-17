#include <cstdlib>
#include "tinylog.hpp"

class Settings {
    public:
        const char* env = std::getenv("SERVER_PORT");
        const char* database_url = std::getenv("DATABASE_URL");
        const char* host = std::getenv("HOST");
        const char* port = std::getenv("PORT");
        const char* log_level = std::getenv("LOG_LEVEL");
        const char* log_format = std::getenv("LOG_FORMAT");
        const char* log_datefmt = std::getenv("LOG_DATEFMT");

    private:


}