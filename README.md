# LLM-SDK

一个基于 C++17 的 AI Chat SDK，封装了大语言模型（LLM）的调用、会话管理、消息持久化等功能，并提供开箱即用的 HTTP API 服务器和 Web 聊天界面。

## ✨ 特性

- **多模型支持** — 可插拔的 Provider 架构，当前已支持 DeepSeek API（v4-flash / v4-pro）
- **会话管理** — 创建/删除/查询会话，自动持久化到 SQLite
- **流式响应** — 支持 SSE（Server-Sent Events）流式消息推送
- **消息持久化** — 会话和消息自动存储，重启不丢失
- **HTTP RESTful API** — 7 个标准接口，便于前端或其他服务集成
- **Web 聊天界面** — 黑金主题 SPA 聊天前端，支持 Markdown 渲染、代码高亮、一键复制
- **可扩展** — 继承 `LLMProvider` 即可接入任何 LLM API

## 📁 项目结构

```
LLM-access/
├── sdk/                          # 核心 SDK（静态库）
│   ├── include/                  # 头文件
│   │   ├── ChatSDK.h             #   SDK 主入口
│   │   ├── SessionManager.h      #   会话管理器
│   │   ├── LLMManager.h          #   模型管理器
│   │   ├── LLMProvider.h         #   LLM Provider 抽象基类
│   │   ├── DeepseekProvider.h    #   DeepSeek Provider 实现
│   │   ├── DataManager.h         #   SQLite 数据管理
│   │   ├── common.h              #   公共数据结构定义
│   │   └── util/Log.h            #   日志工具
│   └── src/                      # 源码实现
├── ChatServer/                   # HTTP 服务器
│   ├── ChatServer.h / .cpp       #   服务器核心逻辑
│   ├── main.cpp                  #   入口（gflags 配置 + 信号处理）
│   ├── CMakeLists.txt
│   ├── required/                 #   需求文档
│   └── build/www/                #   Web 前端（部署到服务器静态目录）
│       ├── index.html
│       ├── css/style.css
│       └── js/script.js
└── SDKDemo/                      # SDK 使用示例 & 测试
    ├── test.cpp                  #   Google Test 测试用例
    └── CMakeLists.txt
```

## 🏗 架构概览

```
┌─────────────────────────────────────────────────────────┐
│                    Web Frontend (SPA)                    │
│              index.html + CSS + JS                      │
│         Markdown / SSE / Code Highlight                 │
└───────────────────┬─────────────────────────────────────┘
                    │  HTTP REST API
┌───────────────────▼─────────────────────────────────────┐
│                    ChatServer                            │
│    POST /api/session          POST /api/message          │
│    GET  /api/sessions         POST /api/message/async    │
│    GET  /api/models           DELETE /api/session/:id    │
│    GET  /api/session/:id/history                         │
└───────────────────┬─────────────────────────────────────┘
                    │
┌───────────────────▼─────────────────────────────────────┐
│                    ChatSDK (Core)                        │
│  ┌───────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ SessionManager│  │  LLMManager  │  │  DataManager │ │
│  │ (会话生命周期) │  │ (模型路由)   │  │ (SQLite存储) │ │
│  └───────────────┘  └──────┬───────┘  └──────────────┘ │
│                            │                             │
│                   ┌────────▼────────┐                   │
│                   │  DeepSeekProvider│                   │
│                   │ (HTTP Client)    │                   │
│                   └────────┬────────┘                   │
└────────────────────────────┼────────────────────────────┘
                             │  HTTPS
                    ┌────────▼────────┐
                    │  DeepSeek API    │
                    │ api.deepseek.com │
                    └─────────────────┘
```

## 📡 API 接口文档

### 通用说明

- **Base URL:** `http://<host>:<port>`
- **Content-Type:** `application/json`
- **成功响应:** `{"success": true, "message": "...", "data": ...}`
- **错误响应:** `{"success": false, "message": "<错误描述>"}`

---

### 1. 获取模型列表

```http
GET /api/models
```

**响应示例：**
```json
{
  "success": true,
  "message": "Model lists retrieved successfully",
  "data": [
    { "name": "deepseek-v4-flash", "desc": "deepseek-v4-flash model" },
    { "name": "deepseek-v4-pro",   "desc": "deepseek-v4-pro model" }
  ]
}
```

---

### 2. 创建会话

```http
POST /api/session
Content-Type: application/json

{ "model": "deepseek-v4-flash" }
```

**响应示例：**
```json
{
  "success": true,
  "message": "Session created successfully",
  "data": {
    "session_id": "session_1712345678_00000001",
    "model": "deepseek-v4-flash"
  }
}
```

---

### 3. 获取会话列表

```http
GET /api/sessions
```

**响应示例：**
```json
{
  "success": true,
  "message": "Session lists retrieved successfully",
  "data": [
    {
      "id": "session_1712345678_00000001",
      "model": "deepseek-v4-flash",
      "created_at": 1712345678,
      "updated_at": 1712345999,
      "messages_count": 6,
      "first_user_message": "你好，请帮我..."
    }
  ]
}
```

---

### 4. 获取会话历史消息

```http
GET /api/session/{session_id}/history
```

**响应示例：**
```json
{
  "success": true,
  "message": "History messages retrieved successfully",
  "data": [
    {
      "id": "message_1712345678_00000001",
      "role": "user",
      "content": "你好",
      "timestamp": 1712345678
    },
    {
      "id": "message_1712345679_00000002",
      "role": "assistant",
      "content": "你好！有什么可以帮助你的吗？",
      "timestamp": 1712345679
    }
  ]
}
```

---

### 5. 发送消息（同步）

```http
POST /api/message
Content-Type: application/json

{
  "session_id": "session_1712345678_00000001",
  "message": "请介绍一下C++17的新特性"
}
```

**响应示例：**
```json
{
  "success": true,
  "message": "Message sent successfully",
  "data": {
    "session_id": "session_1712345678_00000001",
    "response": "C++17 引入了许多重要的新特性..."
  }
}
```

---

### 6. 发送消息（流式 SSE）

```http
POST /api/message/async
Content-Type: application/json

{
  "session_id": "session_1712345678_00000001",
  "message": "请介绍一下C++17的新特性"
}
```

**响应：** `text/event-stream`（SSE 格式）

```
data: "C++17"
data: " 引入了"
data: " 许多"
data: " 重要的新特性..."
data: [DONE]
```

> 每条 `data:` 行是一个增量文本块，`[DONE]` 表示流结束。客户端需按 SSE 规范逐条解析并拼接。

---

### 7. 删除会话

```http
DELETE /api/session/{session_id}
```

**响应示例：**
```json
{
  "success": true,
  "message": "Session deleted successfully"
}
```

## 🚀 快速开始

以下快速开始基于 `SDKDemo/test.cpp` 中的测试代码，展示了 SDK 的典型用法。

### 环境要求

| 依赖 | 版本 | 说明 |
|------|------|------|
| C++ 编译器 | GCC 9+ / Clang 10+ | 需支持 C++17 |
| CMake | ≥ 3.10 | 构建工具 |
| OpenSSL | 1.1+ | HTTPS 请求 |
| SQLite3 | 3.x | 数据持久化 |
| spdlog | 1.x | 日志库 |
| jsoncpp | 1.x | JSON 解析 |
| Google Test | 1.x | 单元测试 |
| gflags | 2.x | 命令行参数（仅 ChatServer） |

### 安装依赖（Ubuntu/Debian）

```bash
sudo apt update
sudo apt install -y build-essential cmake libssl-dev libsqlite3-dev \
    libgtest-dev libgflags-dev libspdlog-dev libjsoncpp-dev
```

### 编译步骤

```bash
# 进入项目目录
cd LLM-access

# 1. 编译 SDK 静态库
cd sdk && mkdir -p build && cd build
cmake ..
make -j$(nproc)
sudo make install    # 安装到 /usr/local/lib 和 /usr/local/include

# 2. 编译 SDK Demo（可选，用于测试 SDK）
cd ../../SDKDemo && mkdir -p build && cd build
cmake ..
make -j$(nproc)

# 3. 编译 ChatServer
cd ../../ChatServer && mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### 运行

```bash
# 启动 ChatServer（默认端口 8889）
cd ChatServer/build
./ChatServer

# 自定义端口
./ChatServer --port=8080

# 使用配置文件
./ChatServer --flagfile=ChatServerConfig.cfg

# 查看帮助
./ChatServer --help
```

启动后在浏览器打开 `http://localhost:8889` 即可使用 Web 聊天界面。

### SDK 代码示例

以下代码来自 `SDKDemo/test.cpp`，展示了 SDK 的核心用法：

```cpp
#include "ChatSDK.h"
#include "util/Log.h"
#include <iostream>

int main() {
    // 1. 初始化日志
    chat_sdk::Log::init("demo", "stdout", spdlog::level::debug);

    // 2. 创建 SDK 实例
    auto sdk = std::make_shared<chat_sdk::ChatSDK>();

    // 3. 配置 DeepSeek API
    auto apiConfig = std::make_shared<chat_sdk::APIConfig>();
    apiConfig->_model_provider = "deepseek";
    apiConfig->_model_name = "deepseek-v4-flash";
    apiConfig->_model_description = "deepseek-v4-flash model";
    apiConfig->_api_key = "sk-your-api-key";        // 替换为你的 API Key
    apiConfig->_endpoint = "https://api.deepseek.com";
    apiConfig->_max_tokens = 2048;
    apiConfig->_temperature = 0.7;

    // 4. 初始化模型
    std::vector<std::shared_ptr<chat_sdk::APIConfig>> configs = { apiConfig };
    sdk->initModels(configs);

    // 5. 创建会话
    std::string sessionId = sdk->createSession("deepseek-v4-flash");
    std::cout << "会话ID: " << sessionId << std::endl;

    // 6. 发送消息（同步）
    std::string response = sdk->sendMessage(sessionId, "你好，请介绍一下自己");
    std::cout << "AI 回复: " << response << std::endl;

    // 7. 发送消息（流式回调）
    auto callback = [](const std::string& delta, bool isEnd) {
        if (isEnd) {
            std::cout << "\n[流式输出结束]" << std::endl;
        } else {
            std::cout << delta << std::flush;
        }
    };
    sdk->sendMessageWithStream(sessionId, "继续聊聊C++", callback);

    // 8. 获取历史消息
    auto history = sdk->getHistoryMessages(sessionId);
    for (auto& msg : history) {
        std::cout << msg._role << ": " << msg._content << std::endl;
    }

    // 9. 获取所有会话
    auto sessions = sdk->getSessionLists();
    for (auto& id : sessions) {
        std::cout << "会话: " << id << std::endl;
    }

    // 10. 删除会话
    sdk->deleteSession(sessionId);

    return 0;
}
```

### 运行 Demo

```bash
cd LLM-access/SDKDemo/build
./LLMAccessSDKDemo
```

Demo 会创建一个会话，提示你输入消息，显示 AI 回复和历史记录。

## ⚙️ ChatServer 配置

### 命令行参数

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `--host` | `0.0.0.0` | 服务器绑定地址 |
| `--port` | `8889` | 服务器端口 |
| `--log_level` | `INFO` | 日志级别 (INFO/DEBUG/WARNING/ERROR) |
| `--deepseek_v4_flash_api_key` | — | DeepSeek Flash API Key |
| `--deepseek_v4_flash_endpoint` | `https://api.deepseek.com/v1` | Flash 模型 API 端点 |
| `--deepseek_v4_pro_api_key` | — | DeepSeek Pro API Key |
| `--deepseek_v4_pro_endpoint` | `https://api.deepseek.com/v1` | Pro 模型 API 端点 |
| `--deepseek_v4_flash_temperature` | `0.7` | Flash 模型温度 (0.0-2.0) |
| `--deepseek_v4_flash_max_tokens` | `2048` | Flash 模型最大 Token 数 |

### 配置文件

启动时自动生成 `ChatServerConfig.cfg`，也可手动创建：

```cfg
--host=0.0.0.0
--port=8889
--log_level=INFO
--deepseek_v4_flash_api_key=sk-your-key
--deepseek_v4_flash_endpoint=https://api.deepseek.com/v1
--deepseek_v4_pro_api_key=sk-your-key
--deepseek_v4_pro_endpoint=https://api.deepseek.com/v1
```

优先级：**命令行参数 > 配置文件 > 默认值**

## 🔧 扩展新模型

只需继承 `LLMProvider` 抽象基类即可接入新的 LLM API：

```cpp
#include "LLMProvider.h"

class MyProvider : public chat_sdk::LLMProvider {
public:
    bool initModel(const std::map<std::string, std::string>& config) override {
        _apiKey = config.at("api_key");
        _endpoint = config.at("endpoint");
        _isAvailable = true;
        return true;
    }

    bool isAvailable() override { return _isAvailable; }
    std::string getModelName() override { return "my-model"; }
    std::string getModelDescription() override { return "My custom model"; }
    std::string getProviderName() override { return "MyProvider"; }

    std::string sendMessage(
        const std::vector<chat_sdk::Message>& messages,
        const std::map<std::string, std::string>& params) override {
        // 实现你的 API 调用逻辑
        return "response";
    }

    std::string sendMessageStream(
        const std::vector<chat_sdk::Message>& messages,
        const std::map<std::string, std::string>& params,
        std::function<void(const std::string&, bool)> callback) override {
        // 实现你的流式 API 调用逻辑
        callback("chunk1", false);
        callback("chunk2", false);
        callback("", true);  // 流结束
        return "full response";
    }
};
```

然后在 `ChatSDK::registerProviders()` 中注册即可。

## 🌐 Web 前端

内置于 `ChatServer/build/www/`，服务启动后直接通过浏览器访问：

- **黑金主题** — 深色背景 + 金色点缀
- **流式对话** — SSE 实时渲染 AI 回复
- **Markdown 渲染** — 标题、列表、表格、代码块
- **代码高亮** — highlight.js 自动语法着色
- **一键复制** — 代码块右上角复制按钮
- **移动端适配** — 响应式布局 + 抽屉侧边栏

### 前端 API 调用流程

```
1. GET /api/sessions        → 加载左侧会话列表
2. 点击「新建对话」         → GET /api/models → 显示模型选择弹窗
3. 选择模型，点「确定」     → POST /api/session → 创建新会话
4. 输入消息，按 Enter       → POST /api/message/async → SSE 流式接收
5. 点击会话历史条目         → GET /api/session/:id/history → 加载历史消息
6. 点击「...」删除          → DELETE /api/session/:id → 删除会话
```

## 📄 数据结构

```cpp
// 消息
struct Message {
    std::string _message_id;   // 消息 ID
    std::string _role;         // "user" | "assistant"
    std::string _content;      // 消息内容
    std::time_t _timestamp;    // 时间戳
};

// 模型配置
struct ModelConfig {
    std::string _model_name;        // 模型名称
    std::string _model_description; // 模型描述
    std::string _model_provider;    // 提供商
    std::string _endpoint;          // API 端点
    int _max_tokens;                // 最大 Token
    double _temperature;            // 温度参数
};

// API 配置（继承 ModelConfig + API Key）
struct APIConfig : public ModelConfig {
    std::string _api_key;           // API 密钥
};

// 会话
struct Session {
    std::string _session_id;        // 会话 ID
    std::string _model_name;        // 模型名称
    std::vector<Message> _messages; // 消息列表
    std::time_t _created_at;        // 创建时间
    std::time_t _updated_at;        // 更新时间
};
```

## 🗄 数据持久化

所有会话和消息通过 SQLite 存储（`chatDB.db`），程序重启后数据不丢失。数据库包含两张核心表：

- **sessions 表** — 存储会话元信息（ID、模型、时间戳）
- **messages 表** — 存储消息记录（角色、内容、时间），通过 session_id 外键关联

## 📝 License

MIT

---

**项目地址：** [https://github.com/fengjunziya/LLM-SDK](https://github.com/fengjunziya/LLM-SDK)
