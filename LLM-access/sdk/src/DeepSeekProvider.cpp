#include "../include/DeepseekProvider.h"
#include "../include/util/Log.h"
#include <jsoncpp/json/json.h>
#include <httplib.h>


namespace chat_sdk {
     // Initialize the model with the given configuration
    void DeepseekProvider::initModel(const std::map<std::string, std::string>& modelConfig) {
        // Initialize the model with the given configuration
        auto it = modelConfig.find("api_key");
        if (it == modelConfig.end()) {
            // Handle the case where the API key is not found
            ERR("API key not found in model configuration");
            return; 
        }
        _apiKey = it->second;
        it = modelConfig.find("endpoint");
        if (it == modelConfig.end()) {
            // Handle the case where the endpoint is not found
            ERR("Endpoint not found in model configuration");
            return;
        }
        _endpoint = it->second;
        _isAvailable = true; // Set the provider as available after successful initialization
        INF("Model initialized successfully, API Key: {}, Endpoint: {}", _apiKey, _endpoint);
    }

    bool DeepseekProvider::isAvailable()// Check if the provider is available
    {
        return _isAvailable;
    }

    std::string DeepseekProvider::getModelName()// Get the model name
    {
        return "deepseek-v4-flash";
    }
    std::string DeepseekProvider::getModelDescription()// Get the model description
    {
        return "A powerful language model for various NLP tasks.";
    }
    // Send message - All content return
    std::string DeepseekProvider::sendMessage(const std::vector<Message>& messages, const std::map<std::string, std::string>& requestParams)
    {
        //1.check if model is available
        if (!_isAvailable) {
            ERR("Model is not available. Please initialize the model first.");
            return "";
        }

        //2.bulid history messages
        Json::Value messagesArr = Json::Value(Json::arrayValue);
        for (const auto& message : messages) {
            
            Json::Value msg;
            msg["role"] = message._role;
            msg["content"] = message._content;
            messagesArr.append(msg);
        }

        //3.bulid request body
        Json::Value requestBody;
        requestBody["model"] = getModelName();
        requestBody["messages"] = messagesArr;

        double temperature = 0.7; // default temperature
        int maxTokens = 100000; // default max tokens
        requestBody["temperature"] = temperature;
        requestBody["max_tokens"] = maxTokens;
        // for (const auto& param : requestParams) {
        //     requestBody[param.first] = param.second;
        // }


        //4.Json serialize
        Json::StreamWriterBuilder writer;
        std::string requestBodyStr = Json::writeString(writer, requestBody);
        INF("Request Body: {}", requestBodyStr);

        //5.Creat HTTP cilent by httplib
        httplib::Client client(_endpoint.c_str());
        client.set_connection_timeout(30,0);
        client.set_read_timeout(60,0);

        //6. Set up headers
        httplib::Headers headers = {
            {"Authorization", "Bearer " + _apiKey}
        };

        //7.Send POST request to the model endpoint
        auto res = client.Post("/chat/completions", headers, requestBodyStr, "application/json");
        if(!res)
        {
            ERR("Failed to connect to the model endpoint.");
            return "";
        }
        if(res->status != 200)
        {
            ERR("Model endpoint returned error: {}. Response body: {}", res->status, res->body);
            return "";
        }
        INF("Send message successfully. Response body: {}", res->body);

        //8.Parse response
        Json::CharReaderBuilder reader;
        Json::Value responseBody;
        std::string errs;
        std::istringstream resStream(res->body);
        if(!Json::parseFromStream(reader, resStream, &responseBody, &errs))
        {
            ERR("Failed to parse response body: {}", errs);
            return "";
        }
        if(!responseBody.isMember("choices") || !responseBody["choices"].isArray() || responseBody["choices"].empty())
        {
            ERR("Invalid response format: 'choices' field is missing or not an array.");
            return "";
        }
        if(!responseBody["choices"][0].isMember("message") || !responseBody["choices"][0]["message"].isObject() || 
           !responseBody["choices"][0]["message"].isMember("content") || !responseBody["choices"][0]["message"]["content"].isString())
        {
            ERR("Invalid response format: 'message' field is missing or does not contain 'content'.");
            return "";
        }
        std::string replyContent = responseBody["choices"][0]["message"]["content"].asString();
        INF("Reply content: {}", replyContent);
        return replyContent;

    }
    // Send message - Stream content return
    std::string DeepseekProvider::sendMessageStream(const std::vector<Message>& messages, const std::map<std::string, std::string>& requestParams, std::function<void(const std::string&, bool)> callback)
    {
        return "";
    }
}