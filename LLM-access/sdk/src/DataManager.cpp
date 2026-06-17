#include "../include/DataManager.h"
#include "../include/util/Log.h"

namespace chat_sdk {
    // constructor
    DataManager::DataManager(const std::string& dbName)
    : _dbName(dbName)
    , _db(nullptr)
    {
        int ret = sqlite3_open(dbName.c_str(), &_db);
        if (ret != SQLITE_OK) {
            ERR("open db failed, error is {}", sqlite3_errmsg(_db));
            return;
        }
        INF("open db success, db name is {}", dbName);

        if (!initDatabase()) {
            ERR("init database failed");
            return;
        }
        INF("init database success");
    }
    // destructor
    DataManager::~DataManager() {
        if (_db) {
            sqlite3_close(_db);
            _db = nullptr;
        }
        INF("close db success");
    }

    // init database
    bool DataManager::initDatabase() {
        // create table if not exists
        std::string create_sessions_sql = R"(
            CREATE TABLE IF NOT EXISTS sessions (
                session_id TEXT PRIMARY KEY,
                model_name TEXT NOT NULL,
                create_time INTEGER NOT NULL,
                update_time INTEGER NOT NULL
            );
        )";
        // execute create table sql
        if (!executeSQL(create_sessions_sql)) {
            ERR("create table sessions failed");
            return false;
        }
        INF("create table sessions success");


        // create table if not exists
        std::string create_messages_sql = R"(
            CREATE TABLE IF NOT EXISTS messages (
                message_id TEXT PRIMARY KEY,
                session_id TEXT NOT NULL,
                role TEXT NOT NULL,
                content TEXT NOT NULL,
                timestamp INTEGER NOT NULL,
                FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
            );
        )";
        // execute create table sql
        if (!executeSQL(create_messages_sql)) {
            ERR("create table messages failed");
            return false;
        }
        INF("create table messages success");
        return true;
    }
    // execute sql
    bool DataManager::executeSQL(const std::string& sql) {
        if (!_db) {
            ERR("db is null");
            return false;
        }
        char* errMsg = nullptr;
        int ret = sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &errMsg);
        if (ret != SQLITE_OK) {
            ERR("execute sql failed, error is {}", errMsg);
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }
                                        // Session
    // insert session
    bool DataManager::insertSession(const Session& session) {
        std::lock_guard<std::mutex> lock(_mutex);
        std::string insert_sql = R"(
            INSERT INTO sessions (session_id, model_name, create_time, update_time)
            VALUES (?, ?, ?, ?)
        )";

        // prepare statement
        sqlite3_stmt* stmt = nullptr;
        int ret = sqlite3_prepare_v2(_db, insert_sql.c_str(), -1, &stmt, nullptr);
        if (ret != SQLITE_OK) {
            ERR("insert session prepare statement failed, error is {}", sqlite3_errmsg(_db));
            return false;
        }

        // bind parameters
        ret = sqlite3_bind_text(stmt, 1, session._session_id.c_str(), -1, nullptr);
        if (ret != SQLITE_OK) {
            ERR("insert session bind parameter session_id failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }

        ret = sqlite3_bind_text(stmt, 2, session._model_name.c_str(), -1, nullptr);
        if (ret != SQLITE_OK) {
            ERR("insert session bind parameter model_name failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }

        ret = sqlite3_bind_int64(stmt, 3, session._created_at);
        if (ret != SQLITE_OK) {
            ERR("insert session bind parameter create_time failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }

        ret = sqlite3_bind_int64(stmt, 4, session._updated_at);
        if (ret != SQLITE_OK) {
            ERR("insert session bind parameter update_time failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }

        // execute statement
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE) {
            ERR("insert session execute statement failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        INF("insert session success, session id is {}", session._session_id);
        sqlite3_finalize(stmt);
        return true;
    }

    // get session by id
    std::shared_ptr<Session> DataManager::getSession(const std::string& sessionId) const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::string select_sql = R"(
            SELECT session_id, model_name, create_time, update_time
            FROM sessions
            WHERE session_id = ?
        )";
        // prepare statement
        sqlite3_stmt* stmt = nullptr;
        int ret = sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr);
        if (ret != SQLITE_OK) {
            ERR("get session prepare statement failed, error is {}", sqlite3_errmsg(_db));
            return nullptr;
        }

        // bind parameters
        ret = sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, nullptr);
        if (ret != SQLITE_OK) {
            ERR("get session bind parameter session_id failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return nullptr;
        }

        // execute statement
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_ROW) {
            ERR("get session execute statement failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return nullptr;
        }

        // fetch results
        Session session;
        session._session_id = sessionId;
        session._model_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        session._created_at = sqlite3_column_int64(stmt, 2);
        session._updated_at = sqlite3_column_int64(stmt, 3);
        session._messages = getSessionMessages(sessionId);

        INF("get session success, session id is {}", sessionId);
        sqlite3_finalize(stmt);
        return std::make_shared<Session>(session);
    }

    // update session timestamp
    bool DataManager::updateSessionTimestamp(const std::string& sessionId, std::time_t timestamp)
    {
        //std::lock_guard<std::mutex> lock(_mutex);
        std::string update_sql = R"(
            UPDATE sessions
            SET update_time = ?
            WHERE session_id = ?
        )";
        // prepare statement
        sqlite3_stmt* stmt = nullptr;
        int ret = sqlite3_prepare_v2(_db, update_sql.c_str(), -1, &stmt, nullptr);
        if (ret != SQLITE_OK) {
            ERR("update session timestamp prepare statement failed, error is {}", sqlite3_errmsg(_db));
            return false;
        }

        // bind parameters
        ret = sqlite3_bind_int64(stmt, 1, timestamp);
        if (ret != SQLITE_OK) {
            ERR("update session timestamp bind parameter update_time failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }

        ret = sqlite3_bind_text(stmt, 2, sessionId.c_str(), -1, SQLITE_TRANSIENT);
        if (ret != SQLITE_OK) {
            ERR("update session timestamp bind parameter session_id failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }

        // execute statement
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE) {
            ERR("update session timestamp execute statement failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        INF("update session timestamp success, session id is {}", sessionId);
        sqlite3_finalize(stmt);
        return true;
    }

    // delete session by session id
    bool DataManager::deleteSession(const std::string& sessionID)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        // delete session
        std::string delete_sql = R"(
            DELETE FROM sessions
            WHERE session_id = ?
        )";
        // prepare statement
        sqlite3_stmt* stmt = nullptr;
        int ret = sqlite3_prepare_v2(_db, delete_sql.c_str(), -1, &stmt, nullptr);
        if (ret != SQLITE_OK) {
            ERR("delete session prepare statement failed, error is {}", sqlite3_errmsg(_db));
            return false;
        }
        // bind parameters
        ret = sqlite3_bind_text(stmt, 1, sessionID.c_str(), -1, nullptr);
        if (ret != SQLITE_OK) {
            ERR("delete session bind parameter session_id failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        // execute statement
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE) {
            ERR("delete session execute statement failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        INF("delete session success, session id is {}", sessionID);
        sqlite3_finalize(stmt);
        return true;
    }

    // delete all sessions
    bool DataManager::deleteAllSessions()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        // delete all sessions
        std::string delete_sql = R"(
            DELETE FROM sessions
        )";
        // prepare statement
        sqlite3_stmt* stmt = nullptr;
        int ret = sqlite3_prepare_v2(_db, delete_sql.c_str(), -1, &stmt, nullptr);
        if (ret != SQLITE_OK) {
            ERR("delete all sessions prepare statement failed, error is {}", sqlite3_errmsg(_db));
            return false;
        }
        // execute statement
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE) {
            ERR("delete all sessions execute statement failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        INF("delete all sessions success");
        sqlite3_finalize(stmt);
        return true;
    }

    // ger all sessions id
    std::vector<std::string> DataManager::getAllSessionsID()const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::string select_sql = R"(
            SELECT session_id FROM sessions ORDER BY update_time DESC)";

        // prepare statement
        sqlite3_stmt* stmt = nullptr;
        int ret = sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr);
        if (ret != SQLITE_OK) {
            ERR("get all sessions id prepare statement failed, error is {}", sqlite3_errmsg(_db));
            return {};
        }

        // execute statement and fetch results
        std::vector<std::string> sessionIDs;
        while ((ret = sqlite3_step(stmt)) == SQLITE_ROW) {
            sessionIDs.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        }
        if (ret != SQLITE_DONE) {
            ERR("get all sessions id execute statement failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return {};
        }
        INF("get all sessions id success, total {} sessions", sessionIDs.size());
        sqlite3_finalize(stmt);
        return sessionIDs;
    }

    // get all sessions Info
    std::vector<std::shared_ptr<Session>> DataManager::getAllSessions()const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::string select_sql = R"(
            SELECT session_id, model_name, create_time, update_time FROM sessions ORDER BY update_time DESC)";

        // prepare statement
        sqlite3_stmt* stmt = nullptr;
        int ret = sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr);
        if (ret != SQLITE_OK) {
            ERR("get all sessions info prepare statement failed, error is {}", sqlite3_errmsg(_db));
            return {};
        }

        // execute statement and fetch results
        std::vector<std::shared_ptr<Session>> sessions;
        while ((ret = sqlite3_step(stmt)) == SQLITE_ROW) {
            std::shared_ptr<Session> session = std::make_shared<Session>();
            session->_session_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            session->_model_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            session->_created_at = sqlite3_column_int64(stmt, 2);
            session->_updated_at = sqlite3_column_int64(stmt, 3);
            session->_messages = getSessionMessages(session->_session_id);
            sessions.push_back(session);
        }
        if (ret != SQLITE_DONE) {
            ERR("get all sessions info execute statement failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return {};
        }
        INF("get all sessions info success, total {} sessions", sessions.size());
        sqlite3_finalize(stmt);
        return sessions;
    }

    // get all sessions count
    int DataManager::getAllSessionsCount()const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::string select_sql = R"(
            SELECT COUNT(*) FROM sessions)";
        // prepare statement
        sqlite3_stmt* stmt = nullptr;
        int ret = sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr);
        if (ret != SQLITE_OK) {
            ERR("get all sessions count prepare statement failed, error is {}", sqlite3_errmsg(_db));
            return 0;
        }

        // fetch results
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_ROW) {
            ERR("get all sessions count execute statement failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return 0;
        }
        int count = sqlite3_column_int(stmt, 0);
        INF("get all sessions count success, count is {}", count);
        sqlite3_finalize(stmt);
        return count;
    }

                                            //  Message
    // insert message to session
    bool DataManager::insertMessage(const std::string& sessionId, const Message& message) {
        std::lock_guard<std::mutex> lock(_mutex);
        std::string insert_sql = R"(
            INSERT INTO messages (message_id, session_id, role, content, timestamp)
            VALUES (?, ?, ?, ?, ?)";    
        // prepare statement
        sqlite3_stmt* stmt = nullptr;
        int ret = sqlite3_prepare_v2(_db, insert_sql.c_str(), -1, &stmt, nullptr);
        if (ret != SQLITE_OK) {
            ERR("insert message prepare statement failed, error is {}", sqlite3_errmsg(_db));
            return false;
        }
        // bind paramaters
        sqlite3_bind_text(stmt, 1, message._message_id.c_str(), -1, nullptr);
        sqlite3_bind_text(stmt, 2, sessionId.c_str(), -1, nullptr);
        sqlite3_bind_text(stmt, 3, message._role.c_str(), -1, nullptr);
        sqlite3_bind_text(stmt, 4, message._content.c_str(), -1, nullptr);
        sqlite3_bind_int64(stmt, 5, message._timestamp);
        // execute statement
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE) {
            ERR("insert message execute statement failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }

        // update session timestamp to current timestamp
        updateSessionTimestamp(sessionId, message._timestamp);
        INF("insert message success, message id is {}", message._message_id);
        sqlite3_finalize(stmt);
        return true;
    }

    // get history messages
    std::vector<Message> DataManager::getSessionMessages(const std::string& sessionId) const
    {
        std::string sql = R"(
            SELECT message_id, role, content, timestamp
            FROM messages
            WHERE session_id = ?
            ORDER BY timestamp DESC
        )";
        
        // prepare statement
        sqlite3_stmt* stmt = nullptr;
        int ret = sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr);
        if (ret != SQLITE_OK) {
            ERR("get history messages prepare statement failed, error is {}", sqlite3_errmsg(_db));
            return {};
        }
        // bind parameter
        ret = sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, nullptr);
        if (ret != SQLITE_OK) {
            ERR("get history messages bind parameter session_id failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return {};
        }
        // execute statement and fetch results
        std::vector<Message> messages;
        while ((ret = sqlite3_step(stmt)) == SQLITE_ROW) {
            Message message;
            message._message_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            message._role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            message._content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            message._timestamp = sqlite3_column_int64(stmt, 3);
            messages.push_back(message);
        }
        if (ret != SQLITE_DONE) {
            ERR("get history messages execute statement failed, error is {}", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return {};
        }
        INF("get history messages success, total {} messages", messages.size());
        sqlite3_finalize(stmt);
        return messages;
    }
    // delete messages in session
        bool DataManager::deleteSessionMessages(const std::string& sessionId)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::string delete_sql = R"(
                DELETE FROM messages
                WHERE session_id = ?
            )";
            // prepare statement
            sqlite3_stmt* stmt = nullptr;
            int ret = sqlite3_prepare_v2(_db, delete_sql.c_str(), -1, &stmt, nullptr);
            if (ret != SQLITE_OK) {
                ERR("delete messages prepare statement failed, error is {}", sqlite3_errmsg(_db));
                return false;
            }
            // bind parameter
            ret = sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, nullptr);
            if (ret != SQLITE_OK) {
                ERR("delete messages bind parameter session_id failed, error is {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return false;
            }
            // execute statement
            ret = sqlite3_step(stmt);
            if (ret != SQLITE_DONE) {
                ERR("delete messages execute statement failed, error is {}", sqlite3_errmsg(_db));
                sqlite3_finalize(stmt);
                return false;
            }
            INF("delete messages success, session id is {}", sessionId);
            sqlite3_finalize(stmt);
            return true;
        }
    
}
