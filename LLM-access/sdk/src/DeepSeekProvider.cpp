#include "../include/DeepseekProvider.h"
#include "../include/util/Log.h"
#include <jsoncpp/json/json.h>
#include <httplib.h>


namespace chat_sdk {
     // Initialize the model with the given configuration
    bool DeepseekProvider::initModel(const std::map<std::string, std::string>& modelConfig) {
        // Initialize the model with the given configuration

        // get api key
        auto it = modelConfig.find("api_key");
        if (it == modelConfig.end()) {
            // Handle the case where the API key is not found
            ERR("API key not found in model configuration");
            return false; 
        }
        _apiKey = it->second;

        // get endpoint
        it = modelConfig.find("endpoint");
        if (it == modelConfig.end()) {
            // Handle the case where the endpoint is not found
            ERR("Endpoint not found in model configuration");
            return false; 
        }
        _endpoint = it->second;

        // get max tokens
        it = modelConfig.find("max_tokens");
        if (it == modelConfig.end()) {
            // Handle the case where the max tokens is not found
            ERR("Max tokens not found in model configuration");
            return false; 
        }
        _maxTokens = std::stoi(it->second);

        // get temperature
        it = modelConfig.find("temperature");
        if (it == modelConfig.end()) {
            // Handle the case where the temperature is not found
            ERR("Temperature not found in model configuration");
            return false; 
        }
        _temperature = std::stod(it->second);


        _isAvailable = true; // Set the provider as available after successful initialization
        _providerName = "Deepseek";
        INF("Model initialized successfully, Provider: {}, API Key: {}, Endpoint: {}", _providerName, _apiKey, _endpoint);
        return true;
    }

    // Check if the provider is available
    bool DeepseekProvider::isAvailable()// Check if the provider is available
    {
        return _isAvailable;
    }

    // Get the model name
    std::string DeepseekProvider::getModelName()// Get the model name
    {
        return "deepseek-v4-flash";
    }

    // Get the provider name
    std::string DeepseekProvider::getProviderName()// Get the provider name
    {
        return "Deepseek";
    }

    // Get the model description
    std::string DeepseekProvider::getModelDescription()// Get the model description
    {
        return "A powerful language model for various NLP tasks.";
    }

    // Send message - All content return
    std::string DeepseekProvider::sendMessage(const std::vector<Message>& messages, const std::map<std::string, std::string>& requestParams)
    {
        //1.check if model is available
        if (!_isAvailable) {
            ERR("Model is not available. Please initialize the model first.\n");
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

 
        //3.get request parameters
        double temperature = _temperature; // default temperature
        int maxTokens = _maxTokens; // default max tokens
        if(requestParams.find("temperature") != requestParams.end())
        {
            try {
                temperature = std::stod(requestParams.at("temperature"));
            } catch (const std::exception& e) {
                ERR("Invalid temperature value: {}. Using default value: {}\n", requestParams.at("temperature"), temperature);
            }
        }
        if(requestParams.find("max_tokens") != requestParams.end())
        {
            try {
                maxTokens = std::stoi(requestParams.at("max_tokens"));
            } catch (const std::exception& e) {
                ERR("Invalid max_tokens value: {}. Using default value: {}\n", requestParams.at("max_tokens"), maxTokens);
            }
        }

        //4.bulid request body
        Json::Value requestBody;
        requestBody["model"] = getModelName();
        requestBody["messages"] = messagesArr;
        requestBody["temperature"] = temperature;
        requestBody["max_tokens"] = maxTokens;


        //5.Json serialize
        Json::StreamWriterBuilder writer;
        std::string requestBodyStr = Json::writeString(writer, requestBody);
        INF("Request Body: {}\n", requestBodyStr);

        //6.Creat HTTP cilent by httplib
        httplib::Client client(_endpoint.c_str());
        client.set_connection_timeout(30,0);
        client.set_read_timeout(60,0);

        //7. Set up headers
        httplib::Headers headers = {
            {"Authorization", "Bearer " + _apiKey}
        };

        //8.Send POST request to the model endpoint
        auto res = client.Post("/chat/completions", headers, requestBodyStr, "application/json");
        if(!res)
        {
            ERR("Failed to connect to the model endpoint.\n");
            return "";
        }
        if(res->status != 200)
        {
            ERR("Model endpoint returned error: {}. Response body: {}\n", res->status, res->body);
            return "";
        }
        INF("Send message successfully. Response body: {}\n", res->body);

        //9.Parse response
        Json::CharReaderBuilder reader;
        Json::Value responseBody;
        std::string errs;
        std::istringstream resStream(res->body);
        if(!Json::parseFromStream(reader, resStream, &responseBody, &errs))
        {
            ERR("Failed to parse response body: {}\n", errs);
            return "";
        }
        if(!responseBody.isMember("choices") || !responseBody["choices"].isArray() || responseBody["choices"].empty())
        {
            ERR("Invalid response format: 'choices' field is missing or not an array.\n");
            return "";
        }
        if(!responseBody["choices"][0].isMember("message") || !responseBody["choices"][0]["message"].isObject() || 
           !responseBody["choices"][0]["message"].isMember("content") || !responseBody["choices"][0]["message"]["content"].isString())
        {
            ERR("Invalid response format: 'message' field is missing or does not contain 'content'.\n");
            return "";
        }
        std::string replyContent = responseBody["choices"][0]["message"]["content"].asString();
        //INF("Reply content: {}\n", replyContent);
        return replyContent;

    }
    // Send message - Stream content return
    std::string DeepseekProvider::sendMessageStream(const std::vector<Message>& messages,
                                                    const std::map<std::string, std::string>& requestParams, 
                                                    std::function<void(const std::string&, bool)> callback)
    {
        // 1.check if model is available
        if (!_isAvailable) {
            ERR("Model is not available. Please initialize the model first.\n");
            return "";
        }
        // 2.bulid history messages
        Json::Value messagesArr = Json::Value(Json::arrayValue);
        for (const auto& message : messages) { 
            Json::Value msg;
            msg["role"] = message._role;
            msg["content"] = message._content;
            messagesArr.append(msg);
        }

        // 3.get request parameters
        double temperature = 0.7; // default temperature
        int maxTokens = 100000; // default max tokens
        if(requestParams.find("temperature") != requestParams.end())
        {
            try {
                temperature = std::stod(requestParams.at("temperature"));
            } catch (const std::exception& e) {
                ERR("Invalid temperature value: {}. Using default value: {}\n", requestParams.at("temperature"), temperature);
            }
        }
        if(requestParams.find("max_tokens") != requestParams.end())
        {
            try {
                maxTokens = std::stoi(requestParams.at("max_tokens"));
            } catch (const std::exception& e) {
                ERR("Invalid max_tokens value: {}. Using default value: {}\n", requestParams.at("max_tokens"), maxTokens);
            }
        }
  

        // 4.bulid request body
        Json::Value requestBody;
        requestBody["model"] = getModelName();
        requestBody["messages"] = messagesArr;
        requestBody["temperature"] = temperature;
        requestBody["max_tokens"] = maxTokens;
        requestBody["stream"] = true; // Enable streaming response

        // 5.Json serialize
        Json::StreamWriterBuilder writer;
        std::string requestBodyStr = Json::writeString(writer, requestBody);
        INF("Request Body: {}\n", requestBodyStr);

        // 6.Set up headers
        httplib::Headers headers = {
            {"Authorization", "Bearer " + _apiKey},
            {"Content-Type", "application/json"},
            {"Accept", "text/event-stream"} // Set Accept header for streaming response
        };

        // 7. Creat HTTP cilent by httplib
        httplib::Client client(_endpoint.c_str());
        client.set_connection_timeout(30,0);
        client.set_read_timeout(300,0);

        // 8. Stream value
        std::string buffer;         //缓冲区
        std::string replyContent;   //回复的完整内容
        std::string errorMsg;       //错误信息
        bool isStreamEnd = false;   //是否是流的结束
        bool isError = false;       //是否发生错误
        int statusCode = 0;         //状态码

        //9. Set Request
        httplib::Request req;
        req.method = "POST";
        req.path = "/v1/chat/completions";
        req.body = requestBodyStr;
        req.headers = headers;

        //10. Set Response_handler, when receive response, check if it is error
        req.response_handler = [&](const httplib::Response& res)        
        {
            if(res.status != 200)
            {
                isError = true;
                statusCode = res.status;
                errorMsg = "HTTP status code: " + std::to_string(res.status) + " " + res.body;
                ERR("Failed to send message. Status code: {}, Error message: {}\n", res.status, res.body);
                return false;
            }
            return true;
        };

        //11. Set content_receiver, when receive data, parse it and call callback
        req.content_receiver = [&](const char* data, size_t size, size_t offset, size_t total_size) -> bool
        {
            // if isError
            if(isError)
            {
                return false;
            }
            buffer.append(data, size);
            size_t pos = 0;

            // get data
            while((pos = buffer.find("\n\n", pos)) != std::string::npos)
            {
                std::string line = buffer.substr(0, pos);
                buffer.erase(0, pos + 2);

                if(line.empty() || line == ":")
                {
                    continue;
                }
                if(line.compare(0, 6, "data: ") == 0)
                {
                    std::string data = line.substr(6);

                    if(data == "[DONE]")
                    {
                        isStreamEnd = true;
                        return true;
                    }

                    // parse data
                    Json::Value datajson;
                    Json::CharReaderBuilder readerbuilder;
                    std::string errorms;
                    std::istringstream resStream(data);
                    if(!Json::parseFromStream(readerbuilder, resStream, &datajson, &errorms))
                    {
                        ERR("Failed to parse JSON data: {}\n", data);
                        return false;
                    }
                    if(!datajson.isMember("choices") || !datajson["choices"].isArray() || datajson["choices"].empty())
                    {
                        ERR("Invalid response format: 'choices' field is missing or not an array.\n");
                        return false;
                    }
                    if(!datajson["choices"][0].isMember("delta") || !datajson["choices"][0]["delta"].isObject() || 
                    !datajson["choices"][0]["delta"].isMember("content"))
                    {
                        ERR("Invalid response format: 'delta' field is missing or does not contain 'content'.\n");
                        return false;
                    }
                    std::string deltaContent = datajson["choices"][0]["delta"]["content"].asString();
                    replyContent += deltaContent;
                    callback(deltaContent, false);
                }
            }
            return true;
        };

        // 12. Send request
        auto res = client.send(req);
        if(!res)
        {
            // send request failed
            ERR("Failed to send request, error message: {}\n", to_string(res.error()));
            return "";
        }

        // 13. Check if stream end
        if(!isStreamEnd)
        {
            WRN("Stream end without [DONE]");
            callback("", true);
        }

        INF("replyContent: {}", replyContent);
        return replyContent;
    }
} 