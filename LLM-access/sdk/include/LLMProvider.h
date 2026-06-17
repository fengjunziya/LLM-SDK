#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include "common.h"

namespace chat_sdk{
    struct LLMProvider {
        public:
            virtual bool initModel(const std::map<std::string, std::string>& modelConfig) = 0; // Initialize the model with the given configuration
            virtual bool isAvailable() = 0; // Check if the provider is available
            virtual std::string getModelName() = 0; // Get the model name
            virtual std::string getModelDescription() = 0; // Get the model description
            virtual std::string getProviderName() = 0; // Get the provider name
            // Send message - All content return
            virtual std::string sendMessage(const std::vector<Message>& messages, const std::map<std::string, std::string>& requestParams) = 0;
            // Send message - Stream content return
            virtual std::string sendMessageStream(const std::vector<Message>& messages, const std::map<std::string, std::string>& requestParams, std::function<void(const std::string&, bool)> callback) = 0;

        protected:
            bool _isAvailable = false; // Whether the provider is available
            std::string _apiKey; // API key for authentication
            std::string _endpoint; // Endpoint for API requests
            int _maxTokens = 2048; // Max tokens for response
            double _temperature = 0.7; // Temperature for response generation
            std::string _providerName; // Provider name
            //std::vector<ModelConfig> _modelConfigs;
    };
}