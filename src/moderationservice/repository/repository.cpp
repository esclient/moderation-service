#include "repository/repository.hpp"
#include <iostream>
#include <memory>
#include <string>

ModerationRepository::ModerationRepository(const std::string& database_url)
    : database_url_(database_url) {

    try {
        db_connection_ = std::make_unique<pqxx::connection>(database_url_);
    } catch (const std::exception& e) {
        std::cerr << "Database connection error: " << e.what() << "\n";
        throw;
    }
}

ModerationRepository::~ModerationRepository() = default;

bool ModerationRepository::SaveModerationResult(const ModerationRecord& result) {
    try {
        if (!db_connection_ || !db_connection_->is_open()) {
            std::cerr << "Database connection is not open." << "\n";
            return false;
        }

        pqxx::work transaction(*db_connection_);

        auto time_since_epoch = result.moderated_at.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(time_since_epoch).count();

        pqxx::params params;
        params.append(result.object_id);
        params.append(static_cast<int>(result.object_type));
        params.append(result.text);
        params.append(result.is_flagged);
        params.append(result.reason);
        params.append(seconds);

        transaction
            .exec("INSERT INTO moderation_records (object_id, object_type, text, "
                  "is_flagged, reason, moderated_at) "
                  "VALUES ($1, $2, $3, $4, $5, to_timestamp($6))",
                  params)
            .no_rows();
        transaction.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving moderation result: " << e.what() << "\n";
        return false;
    }
}
