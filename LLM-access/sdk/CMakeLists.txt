cmake_minimum_required(VERSION 3.16)
project(ai_chat_sdk VERSION 0.1.0 LANGUAGES CXX)

# The SDK is a source-built C++17 static library.
add_library(ai_chat_sdk STATIC
    src/ChatSDK.cpp
    src/DataManager.cpp
    src/DeepSeekProvider.cpp
    src/LLMManager.cpp
    src/SessionManager.cpp
    src/util/Log.cpp
)
add_library(LLMSDK::ai_chat_sdk ALIAS ai_chat_sdk)
target_compile_features(ai_chat_sdk PUBLIC cxx_std_17)

find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED IMPORTED_TARGET jsoncpp)
pkg_check_modules(CPPHTTPLIB REQUIRED IMPORTED_TARGET cpp-httplib)

target_include_directories(ai_chat_sdk PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/ai_chat_sdk>
)

# The distro cpp-httplib package exposes a compiled library via pkg-config.
target_compile_definitions(ai_chat_sdk PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
target_link_libraries(ai_chat_sdk
    PUBLIC
        spdlog::spdlog
        SQLite::SQLite3
        OpenSSL::SSL
        OpenSSL::Crypto
    PRIVATE
        PkgConfig::JSONCPP
        PkgConfig::CPPHTTPLIB
)

include(GNUInstallDirs)
install(TARGETS ai_chat_sdk ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ai_chat_sdk
        FILES_MATCHING PATTERN "*.h")
