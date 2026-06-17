#pragma once
#include "common.h"
#include <mutex>
#include <atomic>
#include <memory>
#include "DataManager.h"


namespace chat_sdk {
    // SessionManager class
    class SessionManager {
    public:
        // constructor
        SessionManager(const std::string& dbName = "chatDB.db");
        // create session
        std::string createSession(const std::string& modelName);
        // get session
        std::shared_ptr<Session> getSession(const std::string& sessionId);
        // add message to session
        bool addMessage(const std::string& sessionId, const Message& message);
        // get history messages
        std::vector<Message> getHistoryMessages(const std::string& sessionId) const;
        // delete session
        bool deleteSession(const std::string& sessionId);
        // update timestamp
        void updateTimestamp(const std::string& sessionId);
        // get all sessions list
        std::vector<std::string> getSessionLists() const;
        // clear all sessions
        void clearAllSessions();
        // get session count
        size_t getSessionCount() const;

    private:
        // generate session id  
        std::string generateSessionId();
        // generate message id
        std::string generateMessageId(size_t messageCount);



       private:
        std::unordered_map<std::string, std::shared_ptr<Session>> _sessions; // session map, key is session id, value is session
        mutable std::mutex _mutex; // mutex for thread safety
        std::atomic<size_t> _session_count = {0}; // session count , is not _sessions size, only to generate session id
        DataManager _dataManager; // data manager for session data storage
    };
}