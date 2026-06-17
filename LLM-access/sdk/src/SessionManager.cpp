#include "../include/SessionManager.h"
#include "../include/util/Log.h"
#include <sstream>
#include <iomanip>



namespace chat_sdk {
    // constructor
    SessionManager::SessionManager(const std::string& dbName) 
    :_dataManager(dbName)
    {
        // get all session from database
        auto sessions = _dataManager.getAllSessions();
        for (auto& session : sessions) {
            _sessions[session->_session_id] = session;
        }
    }

    // generate session id , session id format is "session_timestamp_count"
    std::string SessionManager::generateSessionId() {
        _session_count.fetch_add(1);
        std::time_t timestamp = std::time(nullptr);

        std::ostringstream oss;
        oss << "session_" << timestamp << "_" <<std::setw(8) << std::setfill('0') << _session_count;
        return oss.str();
    }
    
    // generate message id , message id format is "message_timestamp_count"
    std::string SessionManager::generateMessageId(size_t messageCount) {
        messageCount++;
        std::time_t timestamp = std::time(nullptr);

        std::ostringstream oss;
        oss << "message_" << timestamp << "_" <<std::setw(8) << std::setfill('0') << messageCount;
        return oss.str();
    }

    // create session
    std::string SessionManager::createSession(const std::string& modelName) {
        _mutex.lock();

        // generate session id
        std::string session_id = generateSessionId();

        // create session
        auto session = std::make_shared<Session>(Session());
        session->_session_id = session_id;
        session->_model_name = modelName;
        session->_created_at = std::time(nullptr);
        session->_updated_at = session->_created_at;

        _sessions[session_id] = session;
        _mutex.unlock();

        // add session to database
        _dataManager.insertSession(*session);
        INF("create session id :{}", session_id);
        return session_id;
    }

    // get session
    std::shared_ptr<Session> SessionManager::getSession(const std::string& sessionId){
        _mutex.lock();
        // find in memory
        auto it = _sessions.find(sessionId);
        if (it != _sessions.end()) {
            _mutex.unlock();
            it->second->_messages = _dataManager.getSessionMessages(sessionId);
            INF("get session id :{}", sessionId);
            return it->second;
        }
        _mutex.unlock();

        // not find in memory, find in database
        auto session = _dataManager.getSession(sessionId);
        if (session != nullptr) {
            _mutex.lock();
            _sessions[sessionId] = session;
            _mutex.unlock();
            INF("get session id :{}", sessionId);
            return session;
        }
        // not find in database
        WRN("not find session id :{}", sessionId);
        return nullptr;
    }

    // add message to session 
    bool SessionManager::addMessage(const std::string& sessionId, const Message& message) {
        _mutex.lock();
        auto it = _sessions.find(sessionId);
        if (it == _sessions.end()) {
            _mutex.unlock();
            WRN("not find session id :{}", sessionId);
            return false;
        }

        // add message to session
        Message t_mes(message._role, message._content, generateMessageId(it->second->_messages.size()),
          std::time(nullptr));
        it->second->_messages.push_back(t_mes);
        // update timestamp
        it->second->_updated_at = t_mes._timestamp;
        INF("add message to session id :{}", sessionId);
        _mutex.unlock();
        // update database
        _dataManager.insertMessage(sessionId, t_mes);


        return true;
    }

    // get history messages
    std::vector<Message> SessionManager::getHistoryMessages(const std::string& sessionId) const {
        // find in memory
        _mutex.lock();
        auto it = _sessions.find(sessionId);
        if (it != _sessions.end()) {
            _mutex.unlock();
            return it->second->_messages;
        }
        _mutex.unlock();
        // not find in memory, find in database
        auto messages = _dataManager.getSessionMessages(sessionId);
        if (!messages.empty()) {
            return messages;
        }
        WRN("not find session id :{}", sessionId);
        return {};
    }

    // delete session
    bool SessionManager::deleteSession(const std::string& sessionId) {
        _mutex.lock();
        auto it = _sessions.find(sessionId);
        if (it == _sessions.end()) {
            _mutex.unlock();
            return false;
        }
        _sessions.erase(it);
        _mutex.unlock();

        // delete session from database
        _dataManager.deleteSession(sessionId);
        INF("delete session id :{}", sessionId);
        return true;
    }

    // update timestamp
    void SessionManager::updateTimestamp(const std::string& sessionId) {
        _mutex.lock();
        auto it = _sessions.find(sessionId);
        if (it == _sessions.end()) {
            _mutex.unlock();
            return;
        }
        it->second->_updated_at = std::time(nullptr);
        _mutex.unlock();
        // update database
        _dataManager.updateSessionTimestamp(sessionId, it->second->_updated_at);
        INF("update timestamp for session id :{}", sessionId); 
        
    }

    // get all sessions list
    std::vector<std::string> SessionManager::getSessionLists() const {
        // 这里我没加database检查，因为我觉得没必要
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::pair<std::string,std::time_t>> session_list;
        for (const auto& session : _sessions) {
            session_list.push_back({session.first, session.second->_updated_at});
        }
        //sort by timestamp and order by desc
        std::sort(session_list.begin(), session_list.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });
        // get session id list
        std::vector<std::string> session_id_list;
        for (const auto& session : session_list) {
            session_id_list.push_back(session.first);
        }
        return session_id_list;
    }

        // clear all sessions
    void SessionManager::clearAllSessions() {
        _mutex.lock();
        _sessions.clear();
        _session_count.store(0);
        _mutex.unlock();

        // delete all sessions from database
        _dataManager.deleteAllSessions();
    }
    // get session count
    size_t SessionManager::getSessionCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _sessions.size();
    }



}

