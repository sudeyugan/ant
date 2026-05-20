cmake_minimum_required(VERSION 3.20)
project(SocietySimulator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 告诉 CMake 寻找我们需要的包
find_package(entt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# 将第三方库链接到我们的主程序上
target_link_libraries(${PROJECT_NAME} PRIVATE 
    EnTT::EnTT 
    imgui::imgui 
    glfw
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)
endif()
