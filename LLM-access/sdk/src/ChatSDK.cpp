#include "../include/ChatSDK.h"
#include "../include/DeepseekProvider.h"
#include "../include/util/Log.h"

namespace chat_sdk {

    bool ChatSDK::initModels(const std::vector<std::shared_ptr<APIConfig>>& apiConfigs)
    {
        // register providers
        registerProviders();

        // init providers
        initProviders(apiConfigs);

        // set initialized flag
        _isInitialized = true;

        return true;
    }

    // create session
    std::string ChatSDK::createSession(const std::string& modelName)
    {
        // check sdk initialized
        if(!_isInitialized)
        {
            ERR("sdk not initialized");
            return "";
        }
        // create session
        std::string sessionId = _sessionManager.createSession(modelName);
        return sessionId;
    }

    // get session
    std::shared_ptr<Session> ChatSDK::getSession(const std::string& sessionId)
    {
        // check sdk initialized
        if(!_isInitialized)
        {
            ERR("sdk not initialized");
            return nullptr;
        }
        // get session
        return _sessionManager.getSession(sessionId);
    }

    // delete session
    bool ChatSDK::deleteSession(const std::string& sessionId)
    {
        // check sdk initialized
        if(!_isInitialized)
        {
            ERR("sdk not initialized");
            return false;
        }
        // delete session
        return _sessionManager.deleteSession(sessionId);
    }

    // get all sessions list
    std::vector<std::string> ChatSDK::getSessionLists() const
    {
        // check sdk initialized
        if(!_isInitialized)
        {
            ERR("sdk not initialized");
            return {};
        }
        // get all sessions list
        return _sessionManager.getSessionLists();
    }

    // get available models
    std::vector<ModelConfig> ChatSDK::getAvailableModels() const
    {
        // check sdk initialized
        if(!_isInitialized)
        {
            ERR("sdk not initialized");
            return {};
        }
        // get available models
        return _llmManager.getAvailableModels();
    }

    // register providers
    void ChatSDK::registerProviders() {
        // register deepseek-v4-flash model
        if(!_llmManager.checkModelAvailability("deepseek-v4-flash"))
        {
            auto deepseek_v4_flash = std::make_unique<DeepseekProvider>();
            _llmManager.providerRegister("deepseek-v4-flash", std::move(deepseek_v4_flash));
            INF("deepseek-v4-flash model registered successed");
        }

        // register deepseek-v4-pro model
        if(!_llmManager.checkModelAvailability("deepseek-v4-pro"))
        {
            auto deepseek_v4_pro = std::make_unique<DeepseekProvider>();
            _llmManager.providerRegister("deepseek-v4-pro", std::move(deepseek_v4_pro));
            INF("deepseek-v4-pro model registered successed");
        }
        
    }

    bool ChatSDK::initProviders(const std::vector<std::shared_ptr<APIConfig>>& apiConfigs) {
        // init providers
        for(auto& apiConfig : apiConfigs)
        {
            if(apiConfig->_model_provider == "deepseek")
            {
                if(apiConfig->_model_name == "deepseek-v4-flash")
                {
                    // init deepseek-v4-flash model
                    std::map<std::string, std::string> params = {
                        {"api_key", apiConfig->_api_key},
                        {"endpoint", apiConfig->_endpoint},
                        {"max_tokens", std::to_string(apiConfig->_max_tokens)},
                        {"temperature", std::to_string(apiConfig->_temperature)}
                    };
                    if(!_llmManager.initModel("deepseek-v4-flash", params))
                    {
                        ERR("deepseek-v4-flash model init failed");
                        continue;
                    }
                    _modelConfigs["deepseek-v4-flash"] = apiConfig;
                }
                else if(apiConfig->_model_name == "deepseek-v4-pro")
                {
                    // init deepseek-v4-pro model
                    std::map<std::string, std::string> params = {
                        {"api_key", apiConfig->_api_key},
                        {"endpoint", apiConfig->_endpoint},
                        {"max_tokens", std::to_string(apiConfig->_max_tokens)},
                        {"temperature", std::to_string(apiConfig->_temperature)}
                    };
                    if(!_llmManager.initModel("deepseek-v4-pro", params))
                    {
                        ERR("deepseek-v4-pro model init failed");
                        continue;
                    }
                    _modelConfigs["deepseek-v4-pro"] = apiConfig;
                }
                else
                {
                    ERR("Unknown model name: %s", apiConfig->_model_name.c_str());
                }
            }
        }
        return true;
    }


    // Send message to session
    std::string ChatSDK::sendMessage(const std::string& sessionId, const std::string& message)
    {
        // check sdk initialized
        if(!_isInitialized)
        {
            ERR("sdk not initialized");
            return "";
        }

        // get session
        auto session = _sessionManager.getSession(sessionId);
        if(!session)
        {
            ERR("session not found: %s", sessionId.c_str());
            return "";
        }

        // get history messages
        Message newMessage = Message("user", message);
        _sessionManager.addMessage(sessionId, newMessage);
        auto historyMessages = _sessionManager.getHistoryMessages(sessionId);
        
        // set request params
        std::map<std::string, std::string> requestParams = {
            {"max_tokens", std::to_string(_modelConfigs[session->_model_name]->_max_tokens)},
            {"temperature", std::to_string(_modelConfigs[session->_model_name]->_temperature)}
        };
        
        // send message to session
        std::string response = _llmManager.sendMessage(session->_model_name, historyMessages, requestParams);

        // add response to history messages
        _sessionManager.addMessage(sessionId, Message("assistant", response));

        // update session timestamp
        _sessionManager.updateTimestamp(sessionId);

        INF("sendMessage: %s", response.c_str());
        return response;
    }

    // Send message to session with stream
    std::string ChatSDK::sendMessageWithStream(const std::string& sessionId, const std::string& message, 
                                            std::function<void(const std::string&,bool)> callback)
    {
        // check sdk initialized
        if(!_isInitialized)
        {
            ERR("sdk not initialized");
            return "";
        }
        
        // get session
        auto session = _sessionManager.getSession(sessionId);
        if(!session)
        {
            ERR("session not found: %s", sessionId.c_str());
            return "";
        }

        // get history messages
        Message newMessage = Message("user", message);
        _sessionManager.addMessage(sessionId, newMessage);
        auto historyMessages = _sessionManager.getHistoryMessages(sessionId);
        
        // set request params
        std::map<std::string, std::string> requestParams = {
            {"max_tokens", std::to_string(_modelConfigs[session->_model_name]->_max_tokens)},
            {"temperature", std::to_string(_modelConfigs[session->_model_name]->_temperature)}
        };
        
        // send message to session with stream
        std::string response = _llmManager.sendMessageWithStream(session->_model_name, historyMessages, requestParams, callback);
        
        // add response to history messages
        _sessionManager.addMessage(sessionId, Message("assistant", response));

        // update session timestamp
        _sessionManager.updateTimestamp(sessionId);

        INF("sendMessageWithStream: %s", response.c_str());
        return response;
    }
}