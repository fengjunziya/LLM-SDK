#include "../include/LLMManager.h"
#include "../include/util/Log.h"

namespace chat_sdk{
    // Register a new LLM provider
    bool LLMManager::providerRegister(const std::string& modelName, std::unique_ptr<LLMProvider> provider){
        //check param
        if(provider == nullptr){
            ERR("provider is nullptr");
            return false;
        }
        _providers[modelName] = std::move(provider);
        _modelConfig[modelName] = ModelConfig{modelName, provider->getModelDescription(), provider->getProviderName()};

        INF("provider Register success ,modelName: {}", modelName);
        return true;
    }
    // Initialize models with configuration
    bool LLMManager::initModels(const std::string& modelName, const std::map<std::string, std::string>& modelConfig){
        //check model exist
        if(_providers.find(modelName) == _providers.end()){
            ERR("model not exist");
            return false;
        }

        //init model
        if(!_providers[modelName]->initModel(modelConfig)){
            ERR("init model failed");
            return false;
        }

        _modelConfig[modelName]._isAvailable = true;
        INF("init model success ,modelName: {}", modelName);
        return true;
    }
    // Get all available models
    std::vector<ModelConfig> LLMManager::getAvailableModels() const
    {
        std::vector<ModelConfig> availableModels;
        for(const auto& model : _modelConfig){
            if(model.second._isAvailable){
                availableModels.push_back(model.second);
            }
        }
        return availableModels;
    }
    // Check model availability
    bool LLMManager::checkModelAvailability(const std::string& modelName) const
    {
        auto it = _modelConfig.find(modelName);
        if(it == _modelConfig.end()){
            ERR("model not exist");
            return false;
        }
        return it->second._isAvailable;
    }
    // Send a message with no stream to the LLM model
    std::string LLMManager::sendMessage(const std::string& modelName, const std::vector<Message>& messages,
                                        const std::map<std::string, std::string>& requestParams){
        //check model exist
        if(!checkModelAvailability(modelName)){
            ERR("model not available");
            return "";
        }
        //send message
        return _providers[modelName]->sendMessage(messages, requestParams);
    }

    // Send a message with stream to the LLM model
    std::string LLMManager::sendMessageWithStream(const std::string& modelName, const std::vector<Message>& messages,
                                                  const std::map<std::string, std::string>& requestParams,
                                                  std::function<void(const std::string&, bool)> callback){
        //check model exist
        if(!checkModelAvailability(modelName)){
            ERR("model not available");
            return "";
        }
        //send message
        return _providers[modelName]->sendMessageStream(messages, requestParams, callback);
    }
}