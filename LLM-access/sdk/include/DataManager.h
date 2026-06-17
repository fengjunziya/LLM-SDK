#pragma once

#include <sqlite3.h>
#include <mutex>
#include <string>
#include <memory>
#include "common.h"
#include <unordered_map>

namespace chat_sdk {
    class DataManager{
    public:
        DataManager(const std::string& dbPath);
        ~DataManager();
        // init database
        bool initDatabase();
        bool executeSQL(const std::string& sql);


                                                  // Session
        // insert session
        bool insertSession(const Session& session);
        // get session
        std::shared_ptr<Session> getSession(const std::string& sessionId) const;
        // update session timestamp
        bool updateSessionTimestamp(const std::string& sessionId, std::time_t timestamp);
        // delete session by session id
        bool deleteSession(const std::string& sessionId);
        // delete all sessions
        bool deleteAllSessions();
        // get all sessions_id
        std::vector<std::string> getAllSessionsID() const;
        // get all sessions info 
        std::vector<std::shared_ptr<Session>> getAllSessions() const;
        // get all sessions count
        int getAllSessionsCount() const;


                                                //  Message
        // insert message to session
        bool insertMessage(const std::string& sessionId, const Message& message);
        // get history messages
        std::vector<Message> getSessionMessages(const std::string& sessionId) const;
        // delete messages in session
        bool deleteSessionMessages(const std::string& sessionId);



    private:
        std::string _dbName;
        sqlite3* _db;
        mutable std::mutex _mutex;
    };
}