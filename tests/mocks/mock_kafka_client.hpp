#pragma once

#include "config/config.hpp"
#include "kafka/kafkaclient.hpp"
#include "moderation.pb.h"
#include <functional>
#include <vector>

class MockKafkaClient : public KafkaClient {
  public:
    explicit MockKafkaClient(const KafkaConfig& config);

    void Initialize(std::function<void(const moderation::ModerateObjectResponse&, int64_t,
                                       const std::string&, moderation::ObjectType)>
                        callback) override;
    bool SendRequestAsync(const moderation::ModerateObjectRequest& request) override;

    void SetSendResult(bool value) { send_result_ = value; }
    const std::vector<moderation::ModerateObjectRequest>& GetSentRequests() const {
        return sent_requests_;
    }
    void ClearSentRequests() { sent_requests_.clear(); }
    void SimulateResponse(const moderation::ModerateObjectResponse& response, int64_t request_id,
                          const std::string& original_text, moderation::ObjectType type);

  private:
    std::function<void(const moderation::ModerateObjectResponse&, int64_t, const std::string&,
                       moderation::ObjectType)>
        callback_;
    std::vector<moderation::ModerateObjectRequest> sent_requests_;
    bool send_result_ = true;
};