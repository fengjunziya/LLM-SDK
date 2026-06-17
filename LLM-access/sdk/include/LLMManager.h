#pragma once
#include <map>
#include <memory>
#include "LLMProvider.h"
#include "common.h"


namespace chat_sdk{
    class LLMManager{
        public:
            // Register a new LLM provider
            bool providerRegister(const std::string& modelName, std::unique_ptr<LLMProvider> provider);
            // Initialize models with configuration
            bool initModel(const std::string& modelName, const std::map<std::string, std::string>& modelConfig);
            // Get all available models
            std::vector<ModelConfig> getAvailableModels() const;
            // Check model availability
            bool checkModelAvailability(const std::string& modelName) const;
            // Send a message with no stream to the LLM model
            std::string sendMessage(const std::string& modelName, const std::vector<Message>& messages,
                                    const std::map<std::string, std::string>& requestParams);
            // Send a message with stream to the LLM model
            std::string sendMessageWithStream(const std::string& modelName, const std::vector<Message>& messages,
                                              const std::map<std::string, std::string>& requestParams,
                                              std::function<void(const std::string&, bool)> callback);
        private:
            std::map<std::string, std::unique_ptr<LLMProvider>> _providers; // LLM providers
            std::map<std::string, ModelConfig> _modelConfig; // Model config
    };
}