#include "repository/repository.hpp"
#include <iostream>

ModerationRepository::ModerationRepository(const std::string& database_url)
    : database_url_(database_url) {
    
    try 
    {
        db_connection_ = std::make_unique<pqxx::connection>(database_url_);
    } catch (const std::exception &e) 
    {
        std::cerr << "Database connection error: " << e.what() << std::endl;
        throw;
    }
}

ModerationRepository::~ModerationRepository() = default;

bool ModerationRepository::SaveModerationResult(const ModerationRecord& result) {
    
    try {

        if(!db_connection_ || !db_connection_->is_open()) {
            std::cerr << "Database connection is not open." << std::endl;
            return false;
        }


        pqxx::work transaction(*db_connection_);

        auto time_since_epoch = result.moderated_at.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(time_since_epoch).count();

        transaction.exec_params(
            "INSERT INTO moderation_records (object_id, object_type, text, is_flagged, reason, moderated_at) "
            "VALUES ($1, $2, $3, $4, $5, to_timestamp($6))",
            result.object_id,
            static_cast<int>(result.object_type),
            result.text,
            result.is_flagged,
            result.reason,
            seconds
        );
        transaction.commit();
        return true;
    } catch (const std::exception &e) {
        std::cerr << "Error saving moderation result: " << e.what() << std::endl;
        return false;
    }
}

std::optional<ModerationRecord> ModerationRepository::GetModerationRecord(int64_t object_id) {

    try{

        if(!db_connection_ || !db_connection_->is_open()) {
            std::cerr << "Database connection is not open." << std::endl;
            return std::nullopt;
        }

        pqxx::read_transaction transaction(*db_connection_);

        auto result = transaction.exec_params(
            "SELECT object_id, object_type, text, is_flagged, reason, moderated_at "
            "FROM moderation_records "
            "WHERE object_id = $1 "
            "ORDER BY moderated_at DESC "
            "LIMIT 1",
            object_id
        );

        if(result.empty()) {
            return std::nullopt;
        }

        auto row = result[0];
        ModerationRecord record;
        record.object_id = row["object_id"].as<int64_t>();
        record.object_type = static_cast<moderation::ObjectType>(row["object_type"].as<int>());
        record.text = row["text"].as<std::string>();
        record.is_flagged = row["is_flagged"].as<bool>();
        record.reason = row["reason"].as<std::string>();

        auto timestamp_seconds = row["moderated_at"].as<int64_t>();
        record.moderated_at = std::chrono::system_clock::time_point(std::chrono::seconds(timestamp_seconds));

        return record;

    }catch(const std::exception& e)
    {
        std::cerr << "Error retrieving moderation record: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::vector<ModerationRecord> ModerationRepository::GetModerationRecordsByType(moderation::ObjectType object_type, int limit) {
    std::vector<ModerationRecord> records;
    
    try{
        if(!db_connection_ || !db_connection_->is_open()){
            std::cerr << "Database connection is not open." << std::endl;
            return records;
        }

        pqxx::read_transaction transaction(*db_connection_);
        int object_type_int = static_cast<int>(object_type);

        auto result = transaction.exec_params(
            "SELECT object_id, object_type, text, is_flagged, reason, moderated_at "
            "FROM moderation_records "
            "WHERE object_type = $1 "
            "ORDER BY moderated_at DESC "
            "LIMIT $2",
            object_type_int,
            limit
        );

        for(const auto& row : result)
        {
            ModerationRecord record;
            record.object_id = row["object_id"].as<int64_t>();
            record.object_type = static_cast<moderation::ObjectType>(row["object_type"].as<int>());
            record.text = row["text"].as<std::string>();
            record.is_flagged = row["is_flagged"].as<bool>();
            record.reason = row["reason"].as<std::string>();

            auto timestamp_seconds = row["moderated_at"].as<int64_t>();
            record.moderated_at = std::chrono::system_clock::time_point(std::chrono::seconds(timestamp_seconds));

            records.push_back(record);
        }
        return records;

    } catch(const std::exception& e)
    {
        std::cerr << "Error retrieving all moderation records: " << e.what() << std::endl;
        return records;
    }
}

