#pragma once 
#include <string>
#include <ctime>
#include <vector>

namespace chat_sdk {

    //message struct
    struct Message {
        std::string _role; // role: "user" or "assistant"
        std::string _content;  // message content
        std::string _message_id; // message id
        std::time_t _timestamp; // message timestamp

        Message(std::string role, std::string content, std::string message_id, std::time_t timestamp) 
            : _role(role), _content(content), _message_id(message_id), _timestamp(timestamp) {}
    };


    //Model configuration struct
    struct ModelConfig {
        std::string _model_name; // model name
        std::string _model_description; // model description
        std::string _model_provider; // model provider
        std::string _endpoint; // model endpoint base URL
        int _max_tokens; // max tokens for response
        float _temperature; // temperature for response generation
        bool _isAvailable = false; // whether the model is available for use

        ModelConfig(std::string model_name = "", std::string model_description = "", std::string model_provider = "", 
            std::string endpoint = "", int max_tokens = 2048, float temperature = 0.7)
            : _model_name(model_name), _model_description(model_description), _model_provider(model_provider),
             _endpoint(endpoint), _max_tokens(max_tokens), _temperature(temperature) {}
    };

    // API Configure
    struct APIConfig : public ModelConfig {
        std::string _api_key; // API key for authentication 
    };

    // session struct
    struct Session {
        std::string _session_id; // session id
        std::string _model_name; // model name
        std::vector<Message> _messages; // messages in the session
        std::time_t _created_at; // session creation time
        std::time_t _updated_at; // session last update time

        Session(std::string session_id, std::string model_name, std::vector<Message> messages, std::time_t created_at, std::time_t updated_at)
            : _session_id(session_id), _model_name(model_name), _messages(messages), _created_at(created_at), _updated_at(updated_at) {}

    };
}