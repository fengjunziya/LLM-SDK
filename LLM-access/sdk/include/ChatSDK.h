#pragma once
#include "common.h"
#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>
#include "SessionManager.h"
#include "LLMManager.h"



namespace chat_sdk {
    // ChatSDK class
    class ChatSDK {
    public:
        // constructor
        ChatSDK() = default;
        // destructor
        ~ChatSDK() = default;
        // init models
        bool initModels(const std::vector<std::shared_ptr<APIConfig>>& apiConfigs);
        // create session
        std::string createSession(const std::string& modelName);
        // delete session
        bool deleteSession(const std::string& sessionId);
        // get session
        std::shared_ptr<Session> getSession(const std::string& sessionId);
        // get all sessions list
        std::vector<std::string> getSessionLists() const;
        // get available models
        std::vector<ModelConfig> getAvailableModels() const;

        // Send message to session
        std::string sendMessage(const std::string& sessionId, const std::string& message);
        // Send message to session with stream
        std::string sendMessageWithStream(const std::string& sessionId, const std::string& message, 
                                   std::function<void(const std::string&,bool)> callback);

        // get history messages
        std::vector<Message> getHistoryMessages(const std::string& sessionId);



private:
        // register providers
        void registerProviders();
        // init providers
        bool initProviders(const std::vector<std::shared_ptr<APIConfig>>& apiConfigs);

private:
        // is initialized
        bool _isInitialized = false;
        // models config
        std::unordered_map<std::string, std::shared_ptr<ModelConfig>> _modelConfigs;
        // session manager
        SessionManager _sessionManager;
        // model manager
        LLMManager _llmManager;
    };
}