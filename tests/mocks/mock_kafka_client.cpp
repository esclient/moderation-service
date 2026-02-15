#include "mocks/mock_kafka_client.hpp"

MockKafkaClient::MockKafkaClient(const KafkaConfig& config) : KafkaClient(config) {}

void MockKafkaClient::Initialize(
    std::function<void(const moderation::ModerateObjectResponse&, int64_t, const std::string&,
                       moderation::ObjectType)>
        callback) {
    callback_ = std::move(callback);
}

bool MockKafkaClient::SendRequestAsync(const moderation::ModerateObjectRequest& request) {
    sent_requests_.push_back(request);
    return send_result_;
}

void MockKafkaClient::SimulateResponse(const moderation::ModerateObjectResponse& response,
                                       int64_t request_id, const std::string& original_text,
                                       moderation::ObjectType type) {
    if (callback_) {
        callback_(response, request_id, original_text, type);
    }
}