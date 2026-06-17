#pragma once 
#include <string>
#include <map>
#include "LLMProvider.h"

namespace chat_sdk{
    struct DeepseekProvider : public LLMProvider {
        public:
            virtual bool initModel(const std::map<std::string, std::string>& modelConfig); // Initialize the model with the given configuration
            virtual bool isAvailable(); // Check if the provider is available
            virtual std::string getModelName(); // Get the model name
            virtual std::string getModelDescription(); // Get the model description
            virtual std::string getProviderName(); // Get the provider name
            // Send message - All content return
            virtual std::string sendMessage(const std::vector<Message>& messages, const std::map<std::string, std::string>& requestParams);
            // Send message - Stream content return
            virtual std::string sendMessageStream(const std::vector<Message>& messages, const std::map<std::string, std::string>& requestParams, std::function<void(const std::string&, bool)> callback);

    };
}