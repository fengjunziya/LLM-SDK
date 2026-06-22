# Cmake version
cmake_minimum_required(VERSION 3.10)

# Set SDK name
project(ai_chat_sdk)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Build type
set(CMAKE_BUILD_TYPE Release)

# Select the source files
file(GLOB_RECURSE SDK_SOURCES "src/*.cpp")
file(GLOB_RECURSE SDK_HEADERS "include/*.h")


# Generate the static library
add_library(LLM-access STATIC ${SDK_SOURCES} ${SDK_HEADERS})

# Set the output directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

# Define the target compile options
target_compile_definitions(LLM-access PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

target_include_directories(LLM-access PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Set the library include directories
link_directories(/usr/local/lib)

# Find the OpenSSL package
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# Link the static library
target_link_libraries(LLM-access PRIVATE spdlog jsoncpp gtest fmt OpenSSL::SSL OpenSSL::Crypto sqlite3)

# Install the static library
install(TARGETS LLM-access ARCHIVE DESTINATION lib)

# Install the header files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include/ai_chat_sdk FILES_MATCHING PATTERN "*.h")